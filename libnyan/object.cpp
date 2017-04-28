// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object.h"

#include <unordered_set>
#include <vector>

#include "database.h"
#include "error.h"
#include "util.h"


namespace nyan {

Object::Object(const Location &location, Database *database)
	:
	location{location},
	database{database} {}


const std::string &Object::get_name() const {
	return this->name;
}


bool Object::is_registered() const {
	return (this->database != nullptr);
}


const Value &Object::get(const std::string &member) {
	Member *obj_member = this->get_member_ptr_rw(member);

	// first, try the cache lookup.
	const Value *cached = obj_member->cache_get();
	if (cached != nullptr) {
		return *cached;
	}

	// first, find origin of the member.
	// linearize the parent classes.
	auto &linearization = this->get_linearization();

	// find the last value assigning with =
	// it sets the base value where we apply the modifications then
	size_t defined_by = 0;
	const Value *base_value = nullptr;
	for (Object *obj : linearization) {
		const Member *obj_member = obj->get_member(member);
		if (obj_member != nullptr) {
			if (obj_member->get_operation() == nyan_op::ASSIGN) {
				base_value = obj_member->get_value_ptr();
				break;
			}
		}
		defined_by += 1;
	}

	// no operator = was found for this member
	// -> no parent assigned a value.
	// TODO: detect this at load time!
	if (defined_by >= linearization.size()) {
		throw Error{"no operator = found for member"};
	}

	// if this object defines the value, return it directly.
	if (defined_by == 0) {
		return *base_value;
	}

	// create a "working copy" of the value
	// the changes will be applied to it now.
	// TODO: move that to the data store
	std::unique_ptr<Value> result = base_value->copy();

	// Walk back and apply the value changes.
	while (true) {
		const Member *change = linearization[defined_by]->get_member_ptr(member);
		if (change != nullptr) {
			result->apply(change);
		}
		if (defined_by == 0) {
			break;
		}
		defined_by -= 1;
	}

	// Remember the data ptr.
	const Value &result_ref = *result.get();

	// Move result to cache,
	obj_member->cache_save(std::move(result));

	// and return the reference.
	return result_ref;
}


/*
 * determine the type of a member.
 * look at the parents and get the member there.
 * if they don't have it, return nullptr.
 */
Type *Object::infer_type(const std::string &member) const {
	// test if the member is already declared in this object
	const Member *obj_member = this->get_member_ptr(member);
	if (obj_member != nullptr) {
		// this object has this member.
		return obj_member->get_type();
	}

	// this object doesn't have the member.
	// determine the type from its parents.
	auto &linearization = this->get_linearization();

	// assumption: all parents must be valid objects.
	// walk over the parents to find the latest type specialization
	for (Object *obj : linearization) {
		const Member *member_ptr = obj->get_member_ptr(member);

		// parent has the member, it has to be valid
		// because of the assumption
		// and the linearization fixes the multi inheritance.
		if (member_ptr != nullptr) {
			return member_ptr->get_type();
		}
	}

	return nullptr;
}


const Type &Object::get_member_type(const std::string &member) const {
	return *this->get_member(member)->get_type();
}


const Member *Object::get_member(const std::string &member) const {
	const Member *ret = this->get_member_ptr(member);

	if (ret == nullptr) {
		std::ostringstream builder;
		builder << "Could not find member '" << member
		        << "' in object '" << this->get_name() << "'" << std::endl;
		throw NameError{this->location, builder.str()};
	}

	return ret;
}


const Member *Object::get_member_ptr(const std::string &member) const {
	auto it = this->members.find(member);
	if (it != std::end(this->members)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


Member *Object::get_member_ptr_rw(const std::string &member) {
	auto it = this->members.find(member);
	if (it != std::end(this->members)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


bool Object::has(const std::string &member) const {
	return this->members.find(member) != std::end(this->members);
}


bool Object::is_child_of(const Object *parent) const {
	if (this == parent) {
		return true;
	}

	auto &linearization = this->get_linearization();

	for (const Object *obj : linearization) {
		if (parent == obj) {
			return true;
		}
	}
	return false;
}


void Object::patch(const Object *top) {
	if (not top->is_patch()) {
		// TODO: also display the `top` location.
		throw TypeError{
			this->location,
			"provided patch doesn't specify <target> "
			"(or has __patch__) member"
		};
	}

	// TODO: store the applied value in a different storage

	// TODO: verify if we're in the patch target set
	// TODO: recalculate linearization of this object if the parents changed.
	this->apply_value(top, nyan_op::PATCH);
}


bool Object::is_patch() const {
	return this->has("__patch__");
}


void Object::apply_value(const Value *value, nyan_op operation) {
	const Object *change = dynamic_cast<const Object *>(value);

	switch (operation) {
	case nyan_op::PATCH:
		this->patch(change);

	default:
		throw Error{"unknown operation requested"};
	}
}


std::unique_ptr<Value> Object::copy() const {
	throw Error{"nyanobject copy"};
}


size_t Object::hash() const {
	return std::hash<std::string>{}(this->name);
}


bool Object::equals(const Value &other) const {
	auto &other_val = dynamic_cast<const Object &>(other);
	return this->get_name() == other_val.get_name();
}


const std::unordered_set<nyan_op> &Object::allowed_operations(nyan_basic_type value_type) const {

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::PATCH,
	};

	if (value_type.primitive_type == nyan_primitive_type::OBJECT) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


const nyan_basic_type &Object::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::OBJECT,
		nyan_container_type::SINGLE,
	};

	return type;
}


const std::vector<Object *> &Object::get_linearization() const {
	return this->linearization;
}


const std::vector<Object *> &Object::generate_linearization() {
	std::unordered_set<Object *> seen;
	return this->linearize_walk(seen);
}


void Object::delete_linearization() {
	this->linearization.clear();
}



/*
 * c3 linearization of cls(a, b, ...):
 * c3(cls) = [cls] + merge(c3(a), c3(b), ..., [a, b, ...])
 *
 * merge: take first head of lists which is not in any tail of all lists.
 * that head can be the first for multiple lists, pick it from all them.
 * if valid, add to output and remove from all lists where it is head.
 * repeat until all lists are empty.
 * if all heads of the lists appear somewhere in a tail,
 * no linearization exists.
 */
const std::vector<Object *> &Object::linearize_walk(std::unordered_set<Object *> &seen) {

	// test for inheritance loops
	if (seen.find(this) != std::end(seen)) {
		throw Error{"c3: recursive inheritance loop detected"};
	} else {
		seen.insert(this);
	}

	// if already calculated, return directly.
	if (this->linearization.size() > 0) {
		return this->linearization;
	}

	// The current object is always the first in the returned list
	this->linearization.push_back(this);

	// Calculate the parent linearization recursively
	std::vector<std::vector<Object *>> par_linearizations;
	for (auto &parent : this->parents) {
		// Recursive call to get the linearization of the parent
		par_linearizations.push_back(parent->linearize_walk(seen));
	}

	// Add the list of parents to the lists to merge
	par_linearizations.push_back(
		std::vector<Object *>{
			std::begin(this->parents),
			std::end(this->parents),
		}
	);

	// Index to start with in each list
	std::vector<size_t> sublists_heads{par_linearizations.size(), 0};

	// Perform the merge of all parent linearizations
	while (true) {
		Object *candidate = nullptr;
		bool candidate_ok = false;
		size_t sublists_available = par_linearizations.size();

		// Try to find a head that is not element of any tail
		for (size_t i = 0; i < par_linearizations.size(); i++) {
			const auto &par_linearization = par_linearizations[i];
			const size_t headpos = sublists_heads[i];

			// The head position has reached the end
			if (headpos >= par_linearization.size()) {
				sublists_available -= 1;
				continue;
			}

			// Pick a head
			candidate = par_linearization[headpos];
			candidate_ok = true;

			// Test if the candidate is in any tail
			for (auto &tail : par_linearizations) {

				// Start one slot after the head
				for (size_t j = headpos + 1; j < tail.size(); j++) {

					// The head is in that tail, so we fail
					if (unlikely(candidate == tail[j])) {
						candidate_ok = false;
						break;
					}
				}

				// Don't try further tails as one already failed.
				if (unlikely(not candidate_ok)) {
					break;
				}
			}

			// The candidate was not in any tail
			if (candidate_ok) {
				break;
			} else {
				// Try the next candidate
				continue;
			}
		}

		// We found a candidate, add it to the return list
		if (candidate_ok) {
			this->linearization.push_back(candidate);

			// Advance all the lists where the candidate was the head
			for (size_t i = 0; i < par_linearizations.size(); i++) {
				const auto &par_linearization = par_linearizations[i];
				const size_t headpos = sublists_heads[i];

				if (headpos < par_linearization.size()) {
					if (par_linearization[headpos] == candidate) {
						sublists_heads[i] += 1;
					}
				}
			}
		}

		// No more sublists have any entry
		if (sublists_available == 0) {
			return this->linearization;
		}
	}

	throw Error{"cyclic hierarchy, c3 linearization impossible"};
}


std::string Object::str() const {
	std::ostringstream builder;

	builder << this->name << "(";
	if (this->parents.size() > 0) {
		builder << util::strjoin<Object *>(
			",", this->parents,
			[](const auto obj) {return obj->name;}
		);
	}
	builder << "):" << std::endl;

	if (this->members.size() == 0) {
		builder << "    pass" << std::endl;
	}
	else {
		for (auto &entry : this->members) {
			builder << "    " << entry.first
			        << entry.second.str()
			        << std::endl;
		}
	}

	return builder.str();
}


std::string Object::repr() const {
	return this->name;
}

} // namespace nyan
