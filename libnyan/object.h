// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "config.h"
#include "error.h"
#include "value/value_holder.h"


namespace nyan {

class ObjectInfo;
class ObjectState;
class Type;
class Value;
class View;


/**
 * Handle for accessing a nyan object independent of time.
 */
class Object {

public:
	Object();
	Object(const fqon_t &name, const std::shared_ptr<View> &origin);
	~Object();

	/**
	 * Return the fully-qualified object name.
	 */
	const fqon_t &get_name() const;

	/**
	 * Return the view this object was retrieved from.
	 */
	const View &get_view() const;

	/**
	 * Get a member value of this object.
	 * This performs tree traversal for value calculations.
	 */
	ValueHolder get(const memberid_t &member, order_t t=DEFAULT_T) const;

	/**
	 * Invokes the get function and then does a cast.
	 * TODO: either return a stored variant or the shared ptr of the holder
	 */
	template <typename V>
	std::shared_ptr<V> get(memberid_t member, order_t t=DEFAULT_T) const {
		try {
			return std::static_pointer_cast<V>(this->get(member, t).get_ptr());
		}
		catch (std::bad_cast &) {
			// TODO: show which it was
			throw APIError{"bad cast of value"};
		}
	}

	/**
	 * Return the parents of the object.
	 */
	const std::vector<fqon_t> &get_parents(order_t t=DEFAULT_T) const;

	/**
	 * Test if this object has a member of given name.
	 */
	bool has(const memberid_t &member, order_t t=DEFAULT_T) const;

	/**
	 * Test if this object is a child of the given parent.
	 * Returns true if parent equals this object.
	 */
	bool extends(fqon_t other_fqon, order_t t=DEFAULT_T) const;

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
	 * Get the patch target.
	 * Currently, there's only one target and it can't be changed.
	 */
	const fqon_t &get_target() const;

	// TODO: event-callback for this object.
	// invoked whenever the member is updated:
	// triggered either by patch application or by notification of parent object to all childs
	// within a view only (as patches propagate down views anyway)
	// registration of the callback is done in the view!
	// on_change(member, std::function<void(order_t, ObjectState, memberid_t)>);

	const std::vector<fqon_t> &get_linearized(order_t t=DEFAULT_T) const;

protected:

	/**
	 * Return the object state for a given time.
	 */
	const std::shared_ptr<ObjectState> &get_raw(order_t t=DEFAULT_T) const;

	/**
	 * View the object was created from.
	 */
	std::shared_ptr<View> origin;

	/**
	 * The name of this object.
	 */
	fqon_t name;
};


} // namespace nyan
