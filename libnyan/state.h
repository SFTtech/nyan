// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "config.h"


namespace nyan {

class ObjectState;
class View;


/**
 * Database state for some point in time.
 * Contains ObjectStates.
 * Has a reference to the previous state.
 */
class State {
public:
	State(const std::shared_ptr<State> &previous_state);

	State();

	/**
	 * Get the object with the given name from this state,
	 * or any of the parent states.
	 */
	const std::shared_ptr<ObjectState> &get_search(const fqon_t &fqon) const;

	/**
	 * Get the object with given name in this state only.
	 */
	const std::shared_ptr<ObjectState> *get_nosearch(const fqon_t &fqon) const;

	/**
	 * Add an object to the state.
	 * This can only be done for the initial state, i.e. there's no previous state.
	 * Why? The database must be filled at some point.
	 */
	ObjectState &add_object(const fqon_t &name, std::shared_ptr<ObjectState> &&obj);

	/**
	 * Add and potentially replace the objects in the storage from the other state.
	 */
	void update(std::shared_ptr<State> &&obj);

	/**
	 * Copy an object from origin to this state.
	 */
	const std::shared_ptr<ObjectState> &copy_object(const fqon_t &name,
	                                                order_t t,
	                                                std::shared_ptr<View> &origin);

	/**
	 * Return the previous state.
	 * The ptr contains nullptr if this is an initial state.
	 */
	const std::shared_ptr<State> &get_previous_state() const;

	/**
	 * String representation of this state.
	 */
	std::string str() const;

private:
	std::unordered_map<fqon_t, std::shared_ptr<ObjectState>> objects;
	std::shared_ptr<State> previous_state;
};

} // nyan
