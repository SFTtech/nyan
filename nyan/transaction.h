// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <exception>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <tuple>
#include <vector>

#include "config.h"
#include "change_tracker.h"


namespace nyan {

class Object;
class State;
class View;


/**
 * Information to update for a view.
 */
struct view_update {

	using linearizations_t = std::vector<std::vector<fqon_t>>;

	using child_map_t = std::unordered_map<fqon_t, std::unordered_set<fqon_t>>;

	/**
	 * All linearizations to update because of the patches.
	 */
	linearizations_t linearizations;

	/**
	 * Maps objects to their new children.
	 */
	child_map_t children;
};


/**
 * New information for a view.
 * * The state that we're gonna build with this transaction
 * * Changes done in the transaction to invalidate caches.
 */
struct view_state {
	std::shared_ptr<View> view;
	std::shared_ptr<State> state;
	ChangeTracker changes;
};


/**
 * Patch transaction
 */
class Transaction {
public:

	Transaction(order_t at, std::shared_ptr<View> &&origin);

	/**
	 * Add a patch to the transaction.
	 * Apply the patch to the target stored in the patch.
	 */
	bool add(const Object &obj);

	/**
	 * Add a patch to the transaction.
	 * Apply the patch to a custom target, which must be a
	 * child of the target stored in the patch.
	 */
	bool add(const Object &obj, const Object &target);

	/**
	 * Returns true if the transaction was successful.
	 */
	bool commit();

	/**
	 * Return the exception that caused a transaction failure.
	 */
	const std::exception_ptr &get_exception() const;

protected:
	/**
	 * Merge the new states with an existing one from the view.
	 */
	void merge_changed_states();

	/**
	 * Generate all needed updates.
	 * The vector indices are mapped to the views of the
	 * `states` member at the bottom.
	 */
	std::vector<view_update> generate_updates();

	/**
	 * Track which parents need to be notified
	 * of new childs.
	 * Also builds up a list of objects to relinearize
	 * because its parents changed.
	 */
	view_update::child_map_t
	inheritance_updates(const ChangeTracker &tracker,
	                    const std::shared_ptr<View> &view,
	                    std::unordered_set<fqon_t> &objs_to_linearize) const;

	/**
	 * Generate new linearizations for objects that changed.
	 */
	view_update::linearizations_t
	relinearize_objects(const std::unordered_set<fqon_t> &objs_to_linearize,
	                    const std::shared_ptr<View> &view,
	                    const std::shared_ptr<State> &new_state);


	/**
	 * Apply the gathered state updates in all views.
	 * The update list is destroyed.
	 */
	void update_views(std::vector<view_update> &&updates);

	/**
	 * A non-fatal exception occured, so let the transaction fail.
	 */
	void set_error(std::exception_ptr &&exc);

	/**
	 * Exception holder if something failed during the transaction.
	 */
	std::exception_ptr error;

	/**
	 * True if this transaction can be committed.
	 */
	bool valid;

	/**
	 * Time the transaction will take place.
	 */
	order_t at;

	/**
	 * The views where the transaction will be applied in.
	 */
	std::vector<view_state> states;
};

} // namespace nyan
