// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "config.h"
#include "curve.h"


namespace nyan {


/**
 * Cached information about an object.
 * Speeds up information retrieval.
 */
class ObjectCache {
public:
	// TODO: curve for value cache: memberid_t => curve<valueholder>

	/**
	 * Stores the parent linearization of this object over time.
	 */
	Curve<std::vector<fqon_t>> linearizations;

	/**
	 * Stores the direct children an object has over time.
	 */
	Curve<std::unordered_set<fqon_t>> children;

	/**
	 * TODO rethink this (currently unused)
	 *      it could just be a set!.
	 *
	 * Tracking for latest change of an object.
	 *
	 * This is an optimization for cut at t in the curve:
	 * * find changed objects by analyzing the cut branch for names
	 * * find last change before t for each object (via state.previous ptr)
	 * * update the change to this time in this map
	 * * cut off branch
	 *
	 * This also optimizes the search for an object state:
	 * * it's not in this view state if not in this map
	 *   -> search in the database
	 * * if it's in this map, the time of the last state
	 *   in the curve can easily be looked up:
	 *   no need to walk over potentially thousands of state.previous ptrs
	 */
	std::vector<order_t> changes;
};


} // namespace nyan
