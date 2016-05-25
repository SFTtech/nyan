// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_object.h"

#include <unordered_set>
#include <vector>

#include "nyan_error.h"
#include "nyan_store.h"
#include "nyan_util.h"


namespace nyan {

NyanObject::NyanObject(const NyanLocation &location, NyanStore *store)
	:
	location{location},
	store{store} {}


const std::string &NyanObject::get_name() const {
	return this->name;
}


bool NyanObject::is_registered() const {
	return (this->store != nullptr);
}


const NyanValue &NyanObject::get(const std::string &member) {
	NyanMember *obj_member = this->get_member_ptr_rw(member);

	// first, try the cache lookup.
	const NyanValue *cached = obj_member->cache_get();
	if (cached != nullptr) {
		return *cached;
	}

	// first, find origin of the member.
	// linearize the parent classes.
	auto &linearization = this->get_linearization();

	// find the last value assigning with =
	// it sets the base value where we apply the modifications then
	size_t defined_by = 0;
	const NyanValue *base_value = nullptr;
	for (NyanObject *obj : linearization) {
		const NyanMember *obj_member = obj->get_member(member);
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
		throw NyanError{"no operator = found for member"};
	}

	// if this object defines the value, return it directly.
	if (defined_by == 0) {
		return *base_value;
	}

	// create a "working copy" of the value
	// the changes will be applied to it now.
	std::unique_ptr<NyanValue> result = base_value->copy();

	// Walk back and apply the value changes.
	while (true) {
		const NyanMember *change = linearization[defined_by]->get_member_ptr(member);
		if (change != nullptr) {
			result->apply(change);
		}
		if (defined_by == 0) {
			break;
		}
		defined_by -= 1;
	}

	// Remember the data ptr.
	const NyanValue &result_ref = *result.get();

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
NyanType *NyanObject::infer_type(const std::string &member) const {
	// test if the member is already declared in this object
	const NyanMember *obj_member = this->get_member_ptr(member);
	if (obj_member != nullptr) {
		// this object has this member.
		return obj_member->get_type();
	}

	// this object doesn't have the member.
	// determine the type from its parents.
	auto &linearization = this->get_linearization();

	// assumption: all parents must be valid objects.
	// walk over the parents to find the latest type specialization
	for (NyanObject *obj : linearization) {
		const NyanMember *member_ptr = obj->get_member_ptr(member);

		// parent has the member, it has to be valid
		// because of the assumption
		// and the linearization fixes the multi inheritance.
		if (member_ptr != nullptr) {
			return member_ptr->get_type();
		}
	}

	return nullptr;
}


const NyanType &NyanObject::get_type(const std::string &member) const {
	return *this->get_member(member)->get_type();
}


const NyanMember *NyanObject::get_member(const std::string &member) const {
	const NyanMember *ret = this->get_member_ptr(member);

	if (ret == nullptr) {
		std::ostringstream builder;
		builder << "Could not find member '" << member
		        << "' in object '" << this->get_name() << "'" << std::endl;
		throw NameError{this->location, builder.str()};
	}

	return ret;
}


const NyanMember *NyanObject::get_member_ptr(const std::string &member) const {
	auto it = this->members.find(member);
	if (it != std::end(this->members)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


NyanMember *NyanObject::get_member_ptr_rw(const std::string &member) {
	auto it = this->members.find(member);
	if (it != std::end(this->members)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


bool NyanObject::has(const std::string &member) const {
	return this->members.find(member) != std::end(this->members);
}


bool NyanObject::is_child_of(const NyanObject *parent) const {
	if (this == parent) {
		return true;
	}

	auto &linearization = this->get_linearization();

	for (const NyanObject *obj : linearization) {
		if (parent == obj) {
			return true;
		}
	}
	return false;
}


void NyanObject::patch(const NyanObject *top) {
	if (not top->is_patch()) {
		// TODO: also display the `top` location.
		throw TypeError{
			this->location,
			"provided patch doesn't specify <target> "
			"(or has __patch__) member"
		};
	}
	// TODO: verify if we're in the patch target set
	// TODO: recalculate linearization of this object if the parents changed.
	this->apply_value(top, nyan_op::INVALID);
}


bool NyanObject::is_patch() const {
	return this->has("__patch__");
}


void NyanObject::apply_value(const NyanValue *value, nyan_op operation) {
	const NyanObject *change = dynamic_cast<const NyanObject *>(value);

	switch (operation) {
	case nyan_op::PATCH:
		this->patch(change);

	default:
		throw NyanError{"unknown operation requested"};
	}
}


std::unique_ptr<NyanValue> NyanObject::copy() const {
	throw NyanError{"nyanobject copy"};
}


size_t NyanObject::hash() const {
	return std::hash<std::string>{}(this->name);
}


bool NyanObject::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanObject &>(other);
	return this->get_name() == other_val.get_name();
}


const std::unordered_set<nyan_op> &NyanObject::allowed_operations(nyan_type value_type) const {

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::PATCH,
	};

	if (value_type == nyan_type::OBJECT) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


const std::vector<NyanObject *> &NyanObject::get_linearization() const {
	return this->linearization;
}


const std::vector<NyanObject *> &NyanObject::generate_linearization() {
	std::unordered_set<NyanObject *> seen;
	return this->linearize_walk(seen);
}


void NyanObject::delete_linearization() {
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
const std::vector<NyanObject *> &NyanObject::linearize_walk(std::unordered_set<NyanObject *> &seen) {

	// test for inheritance loops
	if (seen.find(this) != std::end(seen)) {
		throw NyanError{"c3: recursive inheritance loop detected"};
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
	std::vector<std::vector<NyanObject *>> par_linearizations;
	for (auto &parent : this->parents) {
		// Recursive call to get the linearization of the parent
		par_linearizations.push_back(parent->linearize_walk(seen));
	}

	// Add the list of parents to the lists to merge
	par_linearizations.push_back(
		std::vector<NyanObject *>{
			std::begin(this->parents),
			std::end(this->parents),
		}
	);

	// Index to start with in each list
	std::vector<size_t> sublists_heads{par_linearizations.size(), 0};

	// Perform the merge of all parent linearizations
	while (true) {
		NyanObject *candidate = nullptr;
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

	throw NyanError{"cyclic hierarchy, c3 linearization impossible"};
}


std::string NyanObject::str() const {
	// TODO: nice string representation
	throw NyanError{"not implemented yet"};
}

} // namespace nyan
