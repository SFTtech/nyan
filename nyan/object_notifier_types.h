// Copyright 2019-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "config.h"


namespace nyan {

class ObjectState;

/**
 * Object change notification callback type.
 * Is called with the change timestamp,
 * the updated object name, and the new nyan object state.
 * TODO: report the changed members (better as a separate type)
 *
 * @param t Time of update.
 * @param fqon Identifier of the updated object.
 * @param state New object state.
 */
using update_cb_t = std::function<void(order_t, const fqon_t &, const ObjectState &)>;

} // namespace nyan
