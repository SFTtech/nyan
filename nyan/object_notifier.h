// Copyright 2019-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "object_notifier_types.h"


namespace nyan {

class View;

/**
 * Notification callback object modification.
 */
class ObjectNotifierHandle {
public:
    ObjectNotifierHandle(const update_cb_t &func);

    /**
     * Calls the user provided function of the notifier.
     *
     * @param t Time of update.
     * @param fqon Identifier of the updated object.
     * @param state New object state.
     */
    void fire(order_t t, const fqon_t &fqon, const ObjectState &state) const;

protected:
    /**
     * The user function which is called when the object is changed.
     */
    update_cb_t func;
};


class ObjectNotifier {
public:

    ObjectNotifier(const fqon_t &fqon,
                   const update_cb_t &func,
                   const std::shared_ptr<View> &view);
    ~ObjectNotifier();

    /**
     * Get the callback handle for the object notifier.
     *
     * @return Shared pointer to the ObjectNotifierHandle.
     */
    const std::shared_ptr<ObjectNotifierHandle> &get_handle() const;

protected:
    /**
     * Which object the notifier is for.
     */
    fqon_t fqon;

    /**
     * View this notifier is active in.
     */
    std::shared_ptr<View> view;

    /**
     * Stores the actual callback handle.
     */
    std::shared_ptr<ObjectNotifierHandle> handle;
};


} // namespace nyan
