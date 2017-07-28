// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <memory>

#include "curve.h"
#include "object.h"
#include "transaction.h"


namespace nyan {

class Database;
class ObjectState;
class State;


/**
 * Database state view.
 */
class View : public std::enable_shared_from_this<View> {
public:
	View(const std::shared_ptr<Database> &database);

	Object get(fqon_t fqon);

	const std::shared_ptr<ObjectState> &get_raw(fqon_t fqon, order_t t=DEFAULT_T);

	Transaction new_transaction(order_t t=DEFAULT_T);

	std::shared_ptr<View> new_child();

	std::vector<std::weak_ptr<View>> &get_children();

	State &new_state(order_t t);

	const State &get_state(order_t t=DEFAULT_T);

	const Database &get_database() const;

protected:
	void add_child(const std::shared_ptr<View> &view);

	/**
	 * Database used if the state curve has no information about
	 * the queried object at all.
	 */
	std::shared_ptr<Database> database;

	/**
	 * Storage of states over time.
	 */
	Curve<std::shared_ptr<State>> history;

	/**
	 * Tracking for latest change of an object.
	 *
	 * This is an optimization for cut at t in the curve:
	 * * find changed objects by analyzing the cut branch for names
	 * * find last change before t for each object (via state.previous ptr)
	 * * update the change to this time in this map
	 * * cut off branch
	 *
	 * This also optimizes the search for an object state:
	 * * it's not in this view state if not in this map
	 *   -> search in the database
	 * * if it's in this map, the time of the last state
	 *   in the curve can easily be looked up:
	 *   no need to walk over potentially thousands of state.previous ptrs
	 */
	std::unordered_map<fqon_t, std::vector<order_t>> changes;

	/**
	 * Child views. Used to propagate down patches.
	 */
	std::vector<std::weak_ptr<View>> children;
};

} // namespace nyan
