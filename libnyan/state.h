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
	State(std::shared_ptr<State> &&previous_state);

	State();

	/**
	 * Get the object with the given name from this state.
	 */
	std::shared_ptr<ObjectState> get(const fqon_t &fqon) const;

	/**
	 * Add an object to the state.
	 * This can only be done for the initial state, i.e. there's no previous state.
	 * Why? The database must be filled at some point.
	 */
	ObjectState &add_object(const fqon_t &name, std::shared_ptr<ObjectState> &&obj);

	/**
	 * Copy an object from origin to this state.
	 */
	ObjectState &copy_object(const fqon_t &name, order_t t, std::shared_ptr<View> &origin);

	/**
	 * String representation of this state.
	 */
	std::string str() const;

private:
	std::unordered_map<fqon_t, std::shared_ptr<ObjectState>> objects;
	std::shared_ptr<State> previous_state;
};

} // nyan
