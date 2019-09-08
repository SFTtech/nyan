// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <deque>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "config.h"
#include "error.h"
#include "value/value_holder.h"
#include "value/object.h"
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
	// This constructor is public, but can't be invoked since the Restricted
	// class is not available. We use this to be able to invoke make_shared
	// within this class, but not outside of it.
	Object(Object::Restricted, const fqon_t &name, const std::shared_ptr<View> &origin)
		: Object(name, origin) {};
	~Object();

	/**
	 * Return the fully-qualified object name.
	 */
	const fqon_t &get_name() const;

	/**
	 * Return the view this object was retrieved from.
	 */
	const std::shared_ptr<View> &get_view() const;

	/**
	 * Get a calculated member value.
	 */
	ValueHolder get(const memberid_t &member, order_t t=LATEST_T) const;

	/**
	 * Invokes the get function and then does a cast.
	 * There's a special variant for T=nyan::Object which creates
	 * an object handle.
	 *
	 * TODO: either return a stored variant reference or the shared ptr of the holder
	 */
	template <typename T>
	std::shared_ptr<T> get(memberid_t member, order_t t=LATEST_T) const;

	/**
	 * Return the parents of the object.
	 */
	const std::deque<fqon_t> &get_parents(order_t t=LATEST_T) const;

	/**
	 * Test if this object has a member of given name.
	 */
	bool has(const memberid_t &member, order_t t=LATEST_T) const;

	/**
	 * Test if this object is a child of the given parent.
	 * Returns true if other_fqon equals this object or any
	 * of its (transitive) parents.
	 */
	bool extends(fqon_t other_fqon, order_t t=LATEST_T) const;

	/**
	 * Return the object metadata.
	 */
	const ObjectInfo &get_info() const;

	/**
	 * Check if this object is a patch.
	 * Currently, a non-patch can never become a new patch.
	 */
	bool is_patch() const;

	/**
	 * Get the patch target. Returns nullptr if the object is not a patch.
	 */
	const fqon_t *get_target() const;

	/**
	 * Return the linearization of this object and its parent objects.
	 */
	const std::vector<fqon_t> &get_linearized(order_t t=LATEST_T) const;

	/**
	 * Register a function that will be called when this object changes in its current view.
	 * It is triggered when a patch is applied on this object
	 * or a parent object.
	 * The callback is registered in this object's view and will be fired as long
	 * as the returned ObjectNotifier was not deleted.
	 */
	std::shared_ptr<ObjectNotifier> subscribe(const update_cb_t &callback);

protected:

	/**
	 * Return the object state for a given time.
	 */
	const std::shared_ptr<ObjectState> &get_raw(order_t t=LATEST_T) const;

	/**
	 * Calculate a member value of this object.
	 * This performs tree traversal for value calculations.
	 */
	ValueHolder calculate_value(const memberid_t &member, order_t t=LATEST_T) const;

	/**
	 * View the object was created from.
	 */
	std::shared_ptr<View> origin;

	/**
	 * The name of this object.
	 */
	fqon_t name;
};


template <typename T>
std::shared_ptr<T> Object::get(memberid_t member, order_t t) const {
	std::shared_ptr<Value> value = this->get(member, t).get_ptr();
	auto ret = std::dynamic_pointer_cast<T>(value);

	if (not ret) {
		std::stringstream ss;
		ss << "failed to fetch value of " << this->name << "." << member
		   << " of real type "
		   << util::typestring(value.get())
		   << " as type " << util::typestring<T>();
		throw APIError{ss.str()};
	}

	return ret;
}


/**
 * Specialization of the get function to generate a nyan::Object
 * from the ObjectValue that is stored in a value.
 */
template <>
std::shared_ptr<Object> Object::get<Object>(memberid_t member, order_t t) const;

} // namespace nyan
