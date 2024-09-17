// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "curve.h"
#include "object.h"
#include "state_history.h"
#include "transaction.h"


namespace nyan {

class Database;
class ObjectState;
class ObjectNotifier;
class ObjectNotifierHandle;
class State;


/**
 * Database state view.
 */
class View : public std::enable_shared_from_this<View> {
	friend class Transaction;

public:
	View(const std::shared_ptr<Database> &database);

	Object get_object(const fqon_t &fqon);

	const std::shared_ptr<ObjectState> &get_raw(const fqon_t &fqon, order_t t = LATEST_T) const;

	const ObjectInfo &get_info(const fqon_t &fqon) const;

	Transaction new_transaction(order_t t = DEFAULT_T);

	std::shared_ptr<View> new_child();

	// TODO: replace by deregistering child when it is destroyed
	void cleanup_stale_children();

	const Database &get_database() const;

	const std::vector<fqon_t> &get_linearization(const fqon_t &fqon, order_t t = LATEST_T) const;

	/**
	 * Get the direct ancestor children of an object.
	 * Does not step further down than one inheritance level.
	 */
	const std::unordered_set<fqon_t> &get_obj_children(const fqon_t &fqon, order_t t = LATEST_T) const;

	/**
	 * Get all ancestor children of an object including the transitive onces.
	 */
	std::unordered_set<fqon_t> get_obj_children_all(const fqon_t &fqon, order_t t = LATEST_T) const;

	/**
	 * Register a function that is called whenever the given object or any of its parents
	 * change a value.
	 * You need to keep the returned ObjectNotifier alive, because when it is deconstructed,
	 * the callback will be deregistered.
	 */
	std::shared_ptr<ObjectNotifier> create_notifier(const fqon_t &fqon, const update_cb_t &callback);

	void deregister_notifier(const fqon_t &fqon,
	                         const std::shared_ptr<ObjectNotifierHandle> &notifier);

	/**
	 * Drop all state later than given time.
	 * This drops child tracking, value caches, linearizations.
	 * Also deletes then-unchanged objects histories.
	 */
	// TODO void reset_from(order_t t=DEFAULT_T);


	/**
	 * Call the notifications for the given objects.
	 */
	void fire_notifications(const std::unordered_set<fqon_t> &changed_objs,
	                        order_t t) const;


protected:
	const std::vector<std::weak_ptr<View>> &get_children();

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

	/**
	 * If this view is a child of another view, this pointer
	 * can bring us back to the parent.
	 */
	std::weak_ptr<View> parent_view;

	/**
	 * Registered event notification callbacks.
	 */
	std::unordered_map<fqon_t, std::unordered_set<std::shared_ptr<ObjectNotifierHandle>>> notifiers;

	// TODO: track transactions and then use tracking to
	//       check for transaction modificationconflicts
	//       beware the child views so that conflicts in them are detected as well
};

} // namespace nyan
