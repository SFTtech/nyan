// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "config.h"
#include "curve.h"


namespace nyan {


/**
 * Cached information about an object.
 * Speeds up information retrieval.
 */
class ObjectHistory {
public:
	/**
	 * Insert a new change record for this object.
	 * This only updates the change history,
	 * not the linearizations or the child tracking.
	 */
	void insert_change(const order_t t);

	/**
	 * Return (in_history, exact_order) for a given order.
	 * in_history:  true if the timestamp can be used to query change
	 *              curves at exact points of order
	 * exact_order: the ordering timestamp for the last matching change.
	 *              will be 0, but invalid, if in_history is false.
	 *
	 * TODO: use std::optional...
	 */
	std::pair<bool, order_t> last_change_before(order_t t) const;

	// TODO: curve for value cache: memberid_t => curve<valueholder>

	/**
	 * Stores the parent linearization of this object over time.
	 */
	Curve<std::vector<fqon_t>> linearizations;

	/**
	 * Stores the direct children an object has over time.
	 */
	Curve<std::unordered_set<fqon_t>> children;

protected:
	/**
	 * History of order points where this object was modified.
	 * This is used to quickly find the matching order for an
	 * object state in the state history.
	 */
	std::set<order_t> changes;
};


} // namespace nyan
