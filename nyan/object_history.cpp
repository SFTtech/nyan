// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_history.h"

#include "compiler.h"


namespace nyan {


void ObjectHistory::insert_change(const order_t time) {
    auto it = this->changes.lower_bound(time);

    // remove all newer entries
    this->changes.erase(it, std::end(this->changes));

    auto ret = this->changes.insert(time);
    if (unlikely(ret.second == false)) {
        throw InternalError{"did not insert change point, it existed before"};
    }
}


std::optional<order_t> ObjectHistory::last_change_before(order_t t) const {
    // get the iterator to the first element greater than t
    auto it = this->changes.upper_bound(t);
    if (it == std::begin(this->changes)) {
        // the requested ordering point is not in this history
        return {};
    }

    // go one back, this is the item we're looking for
    --it;

    return *it;
}

} // namespace nyan
