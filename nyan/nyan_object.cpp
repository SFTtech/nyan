// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_object.h"

#include <unordered_set>
#include <vector>

#include "nyan_store.h"
#include "nyan_util.h"


namespace nyan {

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

	if (seen.find(this) != std::end(seen)) {
		throw "recursive loop detected";
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

	throw "cyclic hierarchy, c3 linearization impossible";
}

} // namespace nyan
