// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
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
     * Get an object state for a given object identifier name.
     *
     * @param fqon Identifier of the object.
     *
     * @return Shared pointer to the object state if it exists, else nullptr.
     */
    const std::shared_ptr<ObjectState> *get(const fqon_t &fqon) const;

    /**
     * Add an object state to the database state. This can only be done for the initial
     * state, i.e. there's no previous state. Why? The database must be filled
     * at some point.
     *
     * @param fqon Identifier of the object.
     * @param obj Shared pointer to the state of the object.
     *
     * @return ObjectState that was added to the state.
     */
    ObjectState &add_object(const fqon_t &name, std::shared_ptr<ObjectState> &&obj);

    /**
     * Add and potentially replace the object states of this state by
     * object states from another database state.
     *
     * @param Shared pointer to the other database state.
     */
    void update(std::shared_ptr<State> &&obj);

    /**
     * Copy an object state from an origin view to this state.
     * If it is in this state already, don't copy it.
     *
     * @param name Identifier of the object.
     * @param t Time for the object state is retrieved.
     * @param origin View from which the object state is copied.
     *
     * @return Shared pointer to the object state.
     */
    const std::shared_ptr<ObjectState> &copy_object(const fqon_t &name,
                                                    order_t t,
                                                    std::shared_ptr<View> &origin);

    /**
     * Get the previous database state.
     *
     * @return Shared pointer to the previous database state,
     *     nullptr if this is the initial state.
     */
    const std::shared_ptr<State> &get_previous_state() const;

    /**
     * Return the object states stored in this state.
     *
     * @return Map of shared pointers to object states by object identifier.
     */
    const std::unordered_map<fqon_t, std::shared_ptr<ObjectState>> &
    get_objects() const;

    /**
     * Get the string representation of this state.
     *
     * @return String representation of this state.
     */
    std::string str() const;

private:
    /**
     * Object states in the database state.
     */
    std::unordered_map<fqon_t, std::shared_ptr<ObjectState>> objects;

    /**
     * Previous state.
     */
    std::shared_ptr<State> previous_state;
};

} // namespace nyan
