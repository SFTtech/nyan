// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <memory>

#include "curve.h"
#include "object.h"
#include "state_history.h"
#include "transaction.h"


namespace nyan {

class Database;
class ObjectState;
class State;


/**
 * Database state view.
 */
class View : public std::enable_shared_from_this<View> {
	friend class Transaction;
public:
	View(const std::shared_ptr<Database> &database);

	Object get(const fqon_t &fqon);

	const std::shared_ptr<ObjectState> &get_raw(const fqon_t &fqon, order_t t=DEFAULT_T) const;

	const ObjectInfo &get_info(const fqon_t &fqon) const;

	Transaction new_transaction(order_t t=DEFAULT_T);

	std::shared_ptr<View> new_child();

	const Database &get_database() const;

	const std::vector<fqon_t> &get_linearization(const fqon_t &fqon, order_t t=DEFAULT_T) const;

	const std::unordered_set<fqon_t> &get_obj_children(const fqon_t &fqon, order_t t=DEFAULT_T) const;

	std::unordered_set<fqon_t> get_obj_children_all(const fqon_t &fqon, order_t t=DEFAULT_T) const;

	/**
	 * Drop all state later than given time.
	 * This drops child tracking, value caches, linearizations.
	 * Also deletes then-unchanged objects histories.
	 */
	// TODO void reset_from(order_t t=DEFAULT_T);


protected:
	std::vector<std::weak_ptr<View>> &get_children();

	void gather_obj_children(std::unordered_set<fqon_t> &target,
	                         const fqon_t &obj,
	                         order_t t) const;

	StateHistory &get_state_history();

	void add_child(const std::shared_ptr<View> &view);

	/**
	 * Database used if the state curve has no information about
	 * the queried object at all.
	 */
	std::shared_ptr<Database> database;

	/**
	 * Data storage over time.
	 */
	StateHistory state;

	/**
	 * Child views. Used to propagate down patches.
	 */
	std::vector<std::weak_ptr<View>> children;

	// TODO: track transactions and then use tracking to
	//       check for transaction modificationconflicts
	//       beware the child views so that conflicts in them are detected as well
};

} // namespace nyan
