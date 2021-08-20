// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
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
 */
struct view_state {
	/**
	 * View where the changes happen.
	 */
	std::shared_ptr<View> view;

	/**
	 * State that we're gonna build with this transaction.
	 */
	std::shared_ptr<State> state;

	/**
	 * Changes done in the transaction to invalidate caches.
	 */
	ChangeTracker changes;
};


/**
 * Patch transaction
 */
class Transaction {
public:

	Transaction(order_t at, std::shared_ptr<View> &&origin);

	/**
	 * Add a patch to the transaction. Apply the patch to the target
	 * stored in the patch.
	 *
	 * @param obj Patch to be applied.
	 *
	 * @return true if the patch is successfully applied, else false.
	 */
	bool add(const Object &obj);

	/**
	 * Add a patch to the transaction. Apply the patch to a custom target,
	 * which must be a descendant of the target stored in the patch.
	 *
	 * @param obj Patch to be applied.
	 * @param target Target object. Must be a descendant of the target stored in the patch.
	 *
	 * @return true if the patch is successfully applied, else false.
	 */
	bool add(const Object &obj, const Object &target);

	/**
	 * Commit the transaction, i.e. update the views.
	 *
	 * @return true if the transaction was successful, else false.
	 */
	bool commit();

	/**
	 * Get the pointer to the exception that caused a transaction failure.
	 *
	 * @return Pointer to the exception that caused a transaction failure.
	 */
	const std::exception_ptr &get_exception() const;

protected:
	/**
	 * Merge the new states with an existing one from the view.
	 */
	void merge_changed_states();

	/**
	 * Generate all needed updates for a commit. The vector indices
	 * are mapped to the views of the \p states member of the transaction.
	 *
	 * @return List of updates for the views.
	 */
	std::vector<view_update> generate_updates();

	/**
	 * Track which parents need to be notified of new children.
	 * Also builds up a list of objects to relinearize because their
	 * parents changed.
	 *
	 * @param[in]  tracker ChangeTracker contaiing the object inheritance changes.
	 * @param[in]  view View for which the objects are changed.
	 * @param[out] objs_to_linearize Identifiers of objects that are relinearized.
	 *
	 * @return Map of sets of children identifiers that need to be notified
	 *     by their parent.
	 */
	view_update::child_map_t
	inheritance_updates(const ChangeTracker &tracker,
	                    const std::shared_ptr<View> &view,
	                    std::unordered_set<fqon_t> &objs_to_linearize) const;

	/**
	 * Generate new linearizations for objects that changed.
	 *
	 * @param objs_to_linearize Identifiers of objects that are relinearized.
	 * @param view View for which the objects are changed.
	 * @param new_state New state of the database after the transaction.
	 *
	 * @return List of list of new object linearizations.
	 */
	view_update::linearizations_t
	relinearize_objects(const std::unordered_set<fqon_t> &objs_to_linearize,
	                    const std::shared_ptr<View> &view,
	                    const std::shared_ptr<State> &new_state);


	/**
	 * Apply the gathered state updates in all views.
	 * The update list is destroyed.
	 *
	 * @param updates List of updates for the views.
	 */
	void update_views(std::vector<view_update> &&updates);

	/**
	 * Set the transaction to invalid and store the error. This can
	 * be used to make the transaction silently fail on non-fatal
	 * errors.
	 *
	 * @param exc Pointer to the error that occured.
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
	 * Time at which the transaction will be commited.
	 */
	order_t at;

	/**
	 * The views to which the transaction will be applied in.
	 */
	std::vector<view_state> states;
};

} // namespace nyan
