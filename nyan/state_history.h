// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "config.h"
#include "object_history.h"


namespace nyan {

class Database;
class MetaInfo;
class ObjectState;
class State;


/**
 * Object state history tracking.
 */
class StateHistory {
public:
	StateHistory(const std::shared_ptr<Database> &base);

	/**
	 * Get the latest database state at or after a given time.
	 *
	 * @param t Time for which the state is retrieved.
	 *
	 * @return Shared pointer to State at time \p t if it exists, else the next state after that.
	 */
	const std::shared_ptr<State> &get_state(order_t t) const;

	/**
	 * Get the latest database state before a given time.
	 *
	 * @param t Time for which the state is retrieved.
	 *
	 * @return Shared pointer to State before time \p t .
	 */
	const std::shared_ptr<State> &get_state_before(order_t t) const;

	/**
	 * Get the database state at a given time.
	 *
	 * @param t Time for which the state is retrieved.
	 *
	 * @return Shared pointer to State at time \p t if it exists, else nullptr..
	 */
	const std::shared_ptr<State> *get_state_exact(order_t t) const;

	/**
	 * Get an object state at a given time.
	 *
	 * @param fqon Identifier of the object.
	 * @param t Time for which the object state is retrieved.
	 *
	 * @return Shared pointer to the ObjectState at time \p t if
	 *     it exists, else the next state before that.
	 */
	const std::shared_ptr<ObjectState> *get_obj_state(const fqon_t &fqon, order_t t) const;

	/**
	 * Record all changes of a new state in the history.
	 *
	 * @param new_state New state in the database.
	 * @param t Time of insertion.
	 */
	void insert(std::shared_ptr<State> &&new_state, order_t t);

	/**
	 * Record a change to the linearization of an object in its history.
	 *
	 * @param ins New linearization of the object. The first element in
	 *     the list is also the identifier of the object.
	 * @param t Time of insertion.
	 */
	void insert_linearization(std::vector<fqon_t> &&ins, order_t t);

	/**
	 * Get the linearization of an object at a given time.
	 *
	 * @param obj Identifier of the object.
	 * @param t Time for which the object linearization is retrieved.
	 * @param meta_info Metadata information of the database.
	 *
	 * @return C3 linearization of the object.
	 */
	const std::vector<fqon_t> &get_linearization(const fqon_t &obj, order_t t, const MetaInfo &meta_info) const;

	/**
	 * Record a change to the children of an object in its history.
	 *
	 * @param obj Identifier of the object.
	 * @param ins New children of the object.
	 * @param t Time of insertion.
	 */
	void insert_children(const fqon_t &obj, std::unordered_set<fqon_t> &&ins, order_t t);

	/**
	 * Get the children of an object at a given time.
	 *
	 * @param obj Identifier of the object.
	 * @param t Time for which the object children are retrieved.
	 * @param meta_info Metadata information of the database.
	 *
	 * @return List of children of the object.
	 */
	const std::unordered_set<fqon_t> &get_children(const fqon_t &obj, order_t t, const MetaInfo &meta_info) const;

protected:
	/**
	 * Get the object history an an object in the database.
	 *
	 * @param obj Identifier of the object.
	 *
	 * @return Pointer to the ObjectHistory of the object if it exists, else nullptr.
	 */
	ObjectHistory *get_obj_history(const fqon_t &obj);

	/**
	 * Get the object history an an object in the database.
	 *
	 * @param obj Identifier of the object.
	 *
	 * @return Pointer to the ObjectHistory of the object if it exists, else nullptr.
	 */
	const ObjectHistory *get_obj_history(const fqon_t &obj) const;

	/**
	 * Get the object history an an object in the database or create it if
	 * it doesn't exist.
	 *
	 * @param obj Identifier of the object.
	 *
	 * @return Pointer to the ObjectHistory of the object.
	 */
	ObjectHistory &get_create_obj_history(const fqon_t &obj);

	/**
	 * Storage of states over time.
	 */
	Curve<std::shared_ptr<State>> history;

	/**
	 * Information history for each object.
	 * Optimizes searches in the history.
	 */
	std::unordered_map<fqon_t, ObjectHistory> object_obj_hists;
};


} // namespace nyan
