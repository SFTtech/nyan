// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_object.h"

#include <unordered_set>
#include <vector>

#include "nyan_error.h"
#include "nyan_store.h"
#include "nyan_util.h"


namespace nyan {

const std::string &NyanObject::get_name() const {
	return this->name;
}


bool NyanObject::is_registered() const {
	return (this->store != nullptr);
}


const NyanValue *NyanObject::get(const std::string &member) {
	auto it = this->members.find(member);
	if (it == std::end(this->members)) {
		std::ostringstream builder;
		builder << "Could not find member '" << member
		        << "' in object '" << this->get_name() << "'" << std::endl;
		throw NameError{builder.str()};
	}

	// first, try the cache lookup.
	NyanValue *cached = it->second->cache_get();
	if (cached != nullptr) {
		return cached;
	}

	// first, find origin of the member.
	// linearize the parent classes.
	std::unordered_set<NyanObject *> seen;
	auto &linearization = this->linearize(seen);

	// find the last value assigning with =
	// it sets the base value where we apply the modifications then
	size_t defined_by = 0;
	NyanValue *base_value = nullptr;
	for (const NyanObject *obj : linearization) {
		NyanMember *obj_member = obj->get_member(member);
		if (obj_member != nullptr) {
			if (obj_member->get_operation() == nyan_op::ASSIGN) {
				base_value = obj_member->get();
				break;
			}
		}
		defined_by += 1;
	}

	// no operator = was found for this member
	// TODO: detect this at load time!
	if (defined_by >= linearization.size()) {
		throw NyanError{"no operator = found for member"};
	}

	// if this object defines the value, return it directly.
	if (defined_by == 0) {
		return base_value;
	}

	// create a "working copy" of the value
	// the changes will be applied to it now.
	std::unique_ptr<NyanValue> result = base_value->copy();

	// Walk back and apply the value changes.
	while (true) {
		NyanMember *change = linearization[defined_by]->get_member(member);
		if (change != nullptr) {
			result->apply(change);
		}
		if (defined_by == 0) {
			break;
		}
		defined_by -= 1;
	}

	// Save result to cache.
	NyanValue *result_ptr = result.get();
	it->second->cache_save(std::move(result));

	// and return the ptr.
	return result_ptr;
}


NyanMember *NyanObject::get_member(const std::string &member) const {
	auto it = this->members.find(member);
	if (it != std::end(this->members)) {
		return it->second.get();
	}
	else {
		return nullptr;
	}
}


bool NyanObject::has(const std::string &member) const {
	return this->members.find(member) != std::end(this->members);
}


void NyanObject::patch(const NyanObject *top) {
	if (not top->has("__patch__")) {
		throw TypeError{"patch object doesn't have __patch__ member"};
	}
	// TODO: verify if we're in the patch target set
	this->apply_value(top, nyan_op::INVALID);
}


void NyanObject::apply_value(const NyanValue *value, nyan_op operation) {
	throw NyanError{"TODO: implement"};
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
std::vector<NyanObject *> &NyanObject::linearize(std::unordered_set<NyanObject *> &seen) {

	if (not this->is_registered()) {
		// Can't linearize because we store the result in the cache.
		throw NyanError{"c3 linearization only for registered objects"};
	}

	// test for inheritance loops
	if (seen.find(this) != std::end(seen)) {
		throw NyanError{"c3: recursive inheritance loop detected"};
	} else {
		seen.insert(this);
	}

	std::vector<NyanObject *> ret;

	// Cache lookup
	auto cache = this->store->linearizations.find(this);
	if (cache != std::end(this->store->linearizations)) {
		return cache->second;
	}

	// The current object is always the first in the returned list
	ret.push_back(this);

	// Calculate the parent linearization recursively
	std::vector<std::vector<NyanObject *>> par_linearizations;
	for (auto &parent : this->parents) {
		// Recursive call to get the linearization of the parent
		par_linearizations.push_back(parent->linearize(seen));
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
			const auto &linearization = par_linearizations[i];
			const size_t headpos = sublists_heads[i];

			// The head position has reached the end
			if (headpos >= linearization.size()) {
				sublists_available -= 1;
				continue;
			}

			// Pick a head
			candidate = linearization[headpos];
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
			ret.push_back(candidate);

			// Advance all the lists where the candidate was the head
			for (size_t i = 0; i < par_linearizations.size(); i++) {
				const auto &linearization = par_linearizations[i];
				const size_t headpos = sublists_heads[i];

				if (headpos < linearization.size()) {
					if (linearization[headpos] == candidate) {
						sublists_heads[i] += 1;
					}
				}
			}
		}

		// No more sublists have any entry
		if (sublists_available == 0) {
			// Store in cache and return reference to it
			return (this->store->linearizations[this] = std::move(ret));
		}
	}

	throw NyanError{"cyclic hierarchy, c3 linearization impossible"};
}

std::string NyanObject::str() const {
	// TODO: nice string representation
	throw NyanError{"not implemented yet"};
}

} // namespace nyan
