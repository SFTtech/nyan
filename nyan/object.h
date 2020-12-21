// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <deque>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "api_error.h"
#include "config.h"
#include "value/set_types.h"
#include "value/value_holder.h"
#include "object_notifier_types.h"
#include "util.h"


namespace nyan {

class ObjectInfo;
class ObjectState;
class ObjectNotifier;
class Type;
class Value;
class View;


/**
 * Handle for accessing a nyan object independent of time.
 */
class Object {
    friend class View;
protected:
    /**
     * Create a nyan-object handle. This is never invoked by the user,
     * as handles are generated internally and then handed over.
     */
    Object(const fqon_t &name, const std::shared_ptr<View> &origin);
    class Restricted {};

public:
    /**
     * Default constructor for an invalid nyan::Object.
     */
    Object() = default;

    // This constructor is public, but can't be invoked since the Restricted
    // class is not available. We use this to be able to invoke make_shared
    // within this class, but not outside of it.
    Object(Object::Restricted, const fqon_t &name, const std::shared_ptr<View> &origin)
        : Object(name, origin) {};
    ~Object();

    /**
     * Get the identifier of this object (fully-qualified object name).
     *
     * @return Identifier of this object.
     */
    const fqon_t &get_name() const;

    /**
     * Get the view this object was created in.
     *
     * @return View of this object.
     */
    const std::shared_ptr<View> &get_view() const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * @param member Identifier of the member.
     * @param t Time for which we want to calculate the value.
     *
     * @return ValueHolder with the value of the member.
     */
    ValueHolder get_value(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Invokes the get_value function and then does a cast to type T.
     * There's a special variant for T=nyan::Object which creates
     * an object handle.
     *
     * TODO: either return a stored variant reference or the shared ptr of the holder
     *
     * @tparam Type the value is casted to.
     *
     * @param member Identifier of the member.
     * @param t Time for which we want to calculate the value.
     *
     * @return Shared pointer with the value of the member.
     */
    template <typename T>
    std::shared_ptr<T> get(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to a number type T.
     *
     * @tparam Number type the value is casted to.
     * @tparam Return type of the value.
     *
     * @param member Identifier of the member.
     * @param t Time for which we want to calculate the value.
     *
     * @return Value of the member with type \p ret.
     */
    template<typename T, typename ret=typename T::storage_type>
    ret get_number(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to int.
     *
     * @param member Identifier of the member.
     * @param t Time for which we want to calculate the value.
     *
     * @return Value of the member.
     */
    value_int_t get_int(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to float.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    value_float_t get_float(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to std::string.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    const std::string &get_text(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to bool.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    bool get_bool(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to std::unordered_set.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    const set_t &get_set(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to nyan::OrderedSet.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    const ordered_set_t &get_orderedset(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Casts to std::string.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    const std::string &get_file(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * Returns an Object.
     *
     * @param member Identifier of the member.
     * @param t Time for which the value is calculated.
     *
     * @return Value of the member.
     */
    Object get_object(const memberid_t &fqon, order_t t=LATEST_T) const;

    /**
     * Get the parents of this object at a given time.
     *
     * @param t Time for which the parents are returned.
     *
     * @return Double-linked queue containing the parents of this object.
     */
    const std::deque<fqon_t> &get_parents(order_t t=LATEST_T) const;

    /**
     * Test if this object has a member with a given name at a given time.
     *
     * @param member Identifier of the member.
     * @param t Time for which the member existence is checked.
     *
     * @return true if the member exists for this object, else false.
     */
    bool has(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * Check if this object is a child of the given parent at a given time.
     *
     * @param other_fqon Identifier of the suspected parent object.
     * @param t Time for which the relationship is checked.
     *
     * @return true if the parent's identifier equals this object's
     *     identifier or that of any of its (transitive) parents,
     *     else false
     */
    bool extends(fqon_t other_fqon, order_t t=LATEST_T) const;

    /**
     * Get the metadata information object for this object.
     *
     * @return Metadata information object for this object.
     */
    const ObjectInfo &get_info() const;

    /**
     * Check if this object is a patch.
     *
     * @return true if this is a patch, else false.
     */
    bool is_patch() const;

    /**
     * Get the identifier of the patch target.
     *
     * @return Identifier of the patch target if this object is a
     *     patch, else nullptr.
     */
    const fqon_t *get_target() const;

    /**
     * Return the C3 linearization of this object at a given time.
     *
     * @param t Time for which the C3 linearization is calculated.
     *
     * @return C3 linearization of this object.
     */
    const std::vector<fqon_t> &get_linearized(order_t t=LATEST_T) const;

    /**
     * Register a function that will be called when this object changes in its current view.
     * It is triggered when a patch is applied on this object or a parent object.
     * The callback is registered in this object's view and will be fired as long
     * as the returned ObjectNotifier was not deleted.
     *
     * @param callback Callback function that is executed when a patch
     *     is applied to this object or a parent object.
     *
     * @return Shared pointer to the ObjectNotifier.
     */
    std::shared_ptr<ObjectNotifier> subscribe(const update_cb_t &callback);

protected:

    /**
     * Get the object state at a given time.
     *
     * @param t Point in time for which the object state is retrieved.
     *
     * @return Shared pointer to the object state.
     */
    const std::shared_ptr<ObjectState> &get_raw(order_t t=LATEST_T) const;

    /**
     * Get the calculated member value for a given member at a given time.
     *
     * @param member Identifier of the member.
     * @param t Time for which we want to calculate the value.
     *
     * @return ValueHolder with the value of the member.
     */
    ValueHolder calculate_value(const memberid_t &member, order_t t=LATEST_T) const;

    /**
     * View the object was created from.
     */
    std::shared_ptr<View> origin;

    /**
     * Identifier of the object.
     */
    fqon_t name;
};


// TODO: use concepts...
template <typename T>
std::shared_ptr<T> Object::get(const memberid_t &member, order_t t) const {
    std::shared_ptr<Value> value = this->get_value(member, t).get_ptr();
    auto ret = std::dynamic_pointer_cast<T>(value);

    if (not ret) {
        throw MemberTypeError{
            this->name,
            member,
            util::typestring(value.get()),
            util::typestring<T>()
        };
    }

    return ret;
}


// TODO: use concepts...
template<typename T, typename ret>
ret Object::get_number(const memberid_t &member, order_t t) const {
    return *this->get<T>(member, t);
}


/**
 * Specialization of the get function to generate a nyan::Object
 * from the ObjectValue that is stored in a value.
 */
template <>
std::shared_ptr<Object> Object::get<Object>(const memberid_t &member, order_t t) const;

} // namespace nyan
