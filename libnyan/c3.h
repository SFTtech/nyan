// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <functional>
#include <unordered_set>
#include <vector>

#include "config.h"
#include "error.h"


namespace nyan {

class ObjectState;


/**
 * Function to fetch an object state.
 */
using objstate_fetch_t = std::function<ObjectState &(const fqon_t &)>;


/**
 * Implements the C3 multi inheritance linearization algorithm
 * to bring the parents of an object into the "right" order.
 */
const std::vector<fqon_t> &linearize(const fqon_t &name, const objstate_fetch_t &get_obj);


/**
 * Recursive walk for the c3 linearization implememtation.
 */
const std::vector<fqon_t> &
linearize_recurse(const fqon_t &name,
                  const objstate_fetch_t &get_obj,
                  std::unordered_set<fqon_t> *seen);


/**
 * Exceptions in the c3 linearization progress.
 */
class C3Error : public Error {
public:
	C3Error(const std::string &msg);
};


} // namespace nyan
