// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "c3.h"

#include "compiler.h"
#include "object_state.h"
#include "util.h"


namespace nyan {


std::vector<fqon_t> linearize(const fqon_t &name, const objstate_fetch_t &get_obj) {
    std::unordered_set<fqon_t> seen;
    return linearize_recurse(name, get_obj, &seen);
}


/*
 * Implementation of c3 inheritance linearization.
 *
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
std::vector<fqon_t>
linearize_recurse(const fqon_t &name,
                  const objstate_fetch_t &get_obj,
                  std::unordered_set<fqon_t> *seen) {

    using namespace std::string_literals;

    // test for inheritance loops
    if (seen->find(name) != std::end(*seen)) {
        throw C3Error{
            "recursive inheritance loop detected: '"s + name + "' already in {"
            + util::strjoin(", ", *seen)
            + "}"
        };
    } else {
        seen->insert(name);
    }

    // get raw ObjectState of this object at the requested time
    const ObjectState &obj_state = get_obj(name);

    // calculate a new linearization in this list
    std::vector<fqon_t> linearization;

    // The current object is always the first in the returned list
    linearization.push_back(name);

    // Get parents of object.
    const auto &parents = obj_state.get_parents();

    // Calculate the parent linearization recursively
    std::vector<std::vector<fqon_t>> par_linearizations;
    par_linearizations.reserve(parents.size() + 1);

    for (auto &parent : parents) {
        // Recursive call to get the linearization of the parent
        par_linearizations.push_back(
            linearize_recurse(parent, get_obj, seen)
        );
    }

    // And at the end, add all parents of this object to the merge-list.
    par_linearizations.push_back(
        {std::begin(parents), std::end(parents)}
    );

    // remove current name from the seen set
    // we only needed it for the recursive call above.
    seen->erase(name);

    // Index to start with in each list
    // On a side note, I used {} instead of () for some time.
    // But that, unfortunately was buggy.
    // What the bug was is left as a fun challenge for the reader.
    std::vector<size_t> sublists_heads(par_linearizations.size(), 0);

    // For each loop, find a candidate to add to the result.
    while (true) {
        const fqon_t *candidate;
        bool candidate_ok = false;
        size_t sublists_available = par_linearizations.size();

        // Try to find a head that is not element of any tail
        for (size_t i = 0; i < par_linearizations.size(); i++) {
            const auto &par_linearization = par_linearizations[i];
            const size_t headpos = sublists_heads[i];

            // The head position has reached the end (i.e. the list is "empty")
            if (headpos >= par_linearization.size()) {
                sublists_available -= 1;
                continue;
            }

            // Pick a head
            candidate = &par_linearization[headpos];
            candidate_ok = true;

            // Test if the candidate is in any tail
            for (size_t j = 0; j < par_linearizations.size(); j++) {

                // The current list will never contain the candidate again.
                if (j == i) {
                    continue;
                }

                const auto &tail = par_linearizations[j];
                const size_t headpos_try = sublists_heads[j];

                // Start one slot after the head
                // and check that the candidate is not in that tail.
                for (size_t k = headpos_try + 1; k < tail.size(); k++) {

                    // The head is in that tail, so we fail
                    if (unlikely(*candidate == tail[k])) {
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
                // Try the next candidate,
                // this means to select the next par_lin list.
                continue;
            }
        }

        // We found a candidate, add it to the return list
        if (candidate_ok) {
            linearization.push_back(*candidate);

            // Advance all the lists where the candidate was the head
            for (size_t i = 0; i < par_linearizations.size(); i++) {
                const auto &par_linearization = par_linearizations[i];
                const size_t headpos = sublists_heads[i];

                if (headpos < par_linearization.size()) {
                    if (par_linearization[headpos] == *candidate) {
                        sublists_heads[i] += 1;
                    }
                }
            }
        }

        // No more sublists have any entry
        if (sublists_available == 0) {
            // linearization is finished!
            return linearization;
        }

        if (not candidate_ok) {
            throw C3Error{
                "Can't find consistent C3 resolution order for "s
                + name + " for bases " + util::strjoin(", ", parents)
            };
        }
    }

    // should not be reached :)
    throw InternalError{"C3 internal error"};
}


C3Error::C3Error(const std::string &msg)
    :
    Error{msg} {}


} // namespace nyan
