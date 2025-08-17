// Copyright 2016-2025 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <deque>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "api_error.h"
#include "concept.h"
#include "config.h"
#include "object_notifier_types.h"
#include "util.h"
#include "value/container_types.h"
#include "value/none.h"
#include "value/value_holder.h"


namespace nyan {

class NumberBase;
class Object;
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
	Object(Object::Restricted, const fqon_t &name, const std::shared_ptr<View> &origin) :
		Object(name, origin) {};
	~Object();

	/**
	 * Get the identifier of this object (fully-qualified object name).
	 *
	 * @return fqon of this object.
	 */
	const fqon_t &get_name() const;

	/**
	 * Get the view of the database this object is associated with
	 *
	 * @return Database view.
	 */
	const std::shared_ptr<View> &get_view() const;

	/**
	 * Get a new value holder that contains the calculated member value
	 * for a given member at a given time.
	 *
	 * @param member Member ID.
	 * @param t Time for which we want to calculate the value.
	 *
	 * @return ValueHolder containing the raw value of the member.
	 */
	ValueHolder get_value(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value container for a given member at a given time.
	 *
	 * Invokes the get_value function and then does a cast to type T which
	 * is a nyan value type.
	 *
	 * @tparam T nyan type of the value.
	 *
	 * @param member Member ID.
	 * @param t Time for which we want to calculate the value.
	 *
	 * @return Value of the member.
	 */
	template <ValueOrObjectLike T>
	std::shared_ptr<T> get(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value container for a given member at a given time.
	 *
	 * This variant of \p get() always explicitely tests if the member value
	 * is \p None (i.e. if there is an optional value).
	 *
	 * @param member Member ID.
	 * @param t Time to retrieve the member for.
	 *
	 * @return Value of the member.
	 */
	template <ValueOrObjectLike T, bool may_be_none = true>
	std::optional<std::shared_ptr<T>> get_optional(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for a number type member (\p int or \p float).
	 *
	 * @tparam Number type of the member.
	 * @tparam Return type of the value.
	 *
	 * @param member Member ID.
	 * @param t Time for which we want to calculate the value.
	 *
	 * @return Value of the member.
	 */
	template <std::derived_from<NumberBase> T, typename ret = typename T::storage_type>
	ret get_number(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p int type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which we want to calculate the value.
	 *
	 * @return Value of the member.
	 */
	value_int_t get_int(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p float type member.
	 *
	 * Note that this actually returns a double.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	value_float_t get_float(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p text type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	std::string get_text(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p bool type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	bool get_bool(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p set type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	set_t get_set(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p orderedset type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	ordered_set_t get_orderedset(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p dict type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	dict_t get_dict(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for an \p file type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	std::string get_file(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value container for an \p object type member.
	 *
	 * @param member Member ID.
	 * @param t Time for which the value is calculated.
	 *
	 * @return Value of the member.
	 */
	Object get_object(const memberid_t &fqon, order_t t = LATEST_T) const;

	/**
	 * Get the parents of this object at a given time.
	 *
	 * @param t Time for which the parents are returned.
	 *
	 * @return Double-linked queue containing the parents of this object.
	 */
	const std::deque<fqon_t> &get_parents(order_t t = LATEST_T) const;

	/**
	 * Test if this object has a member with a given name at a given time.
	 *
	 * @param member Identifier of the member.
	 * @param t Time for which the member existence is checked.
	 *
	 * @return true if the member exists for this object, else false.
	 */
	[[deprecated("Use has_member(..) instead")]]
	bool has(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Check if this object has a member with a given name at a given time.
	 *
	 * @param member Identifier of the member.
	 * @param t Time for which the member existence is checked.
	 *
	 * @return true if the member exists for this object, else false.
	 */
	bool has_member(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * Check if this object is a descendant/child of the given object at a given time.
	 *
	 * @param other_fqon Identifier of the suspected parent/ancestor object.
	 * @param t Time for which the relationship is checked.
	 *
	 * @return true if the ancestors's identifier equals this object's
	 *         identifier or that of any of its (transitive) parents,
	 *         else false
	 */
	bool extends(const fqon_t &other_fqon, order_t t = LATEST_T) const;

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
	const std::vector<fqon_t> &get_linearized(order_t t = LATEST_T) const;

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
	const std::shared_ptr<ObjectState> &get_raw(order_t t = LATEST_T) const;

	/**
	 * Get the calculated member value for a given member at a given time.
	 *
	 * @param member Identifier of the member.
	 * @param t Time for which we want to calculate the value.
	 *
	 * @return ValueHolder with the value of the member.
	 */
	ValueHolder calculate_value(const memberid_t &member, order_t t = LATEST_T) const;

	/**
	 * View the object was created from.
	 */
	std::shared_ptr<View> origin;

	/**
	 * Identifier of the object.
	 */
	fqon_t name;
};


template <ValueOrObjectLike T>
std::shared_ptr<T> Object::get(const memberid_t &member, order_t t) const {
	auto ret = this->get_optional<T, false>(member, t);
	return *ret;
}


template <ValueOrObjectLike T, bool may_be_none>
std::optional<std::shared_ptr<T>> Object::get_optional(const memberid_t &member, order_t t) const {
	std::shared_ptr<Value> value = this->get_value(member, t).get_ptr();
	if constexpr (may_be_none) {
		if (value == None::value) {
			return {};
		}
	}

	auto ret = std::dynamic_pointer_cast<T>(value);

	if (not ret) {
		throw MemberTypeError{
			this->name,
			member,
			util::typestring(value.get()),
			util::typestring<T>()};
	}

	return ret;
}


template <std::derived_from<NumberBase> T, typename ret>
ret Object::get_number(const memberid_t &member, order_t t) const {
	return *this->get<T>(member, t);
}


/**
 * Specialization of the get function to generate a nyan::Object
 * from the ObjectValue that is stored in a value.
 */
template <>
std::shared_ptr<Object> Object::get<Object>(const memberid_t &member, order_t t) const;


/**
 * Specialization of the get_optional function to generate a nyan::Object
 * from the ObjectValue that is stored in a optional value.
 *
 * Note the missing template parameter for `may_be_optional`, it uses the default
 * value from the base template...
 */
template <>
std::optional<std::shared_ptr<Object>> Object::get_optional<Object>(const memberid_t &member, order_t t) const;

} // namespace nyan
