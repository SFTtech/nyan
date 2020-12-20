// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <optional>
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
     *
     * @param t Order of insertion.
     */
    void insert_change(const order_t t);

    /**
     * Get the order of the last change before a given order.
     *
     * @param t Order for which a preceeding order is searched in the object history.
     *
     * @return Order of the last change if there is one, else empty std::optional.
     */
    std::optional<order_t> last_change_before(order_t t) const;

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
