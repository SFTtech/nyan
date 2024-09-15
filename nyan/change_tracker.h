// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "config.h"


namespace nyan {


/**
 * Change tracking for a single object.
 */
class ObjectChanges {
public:
	/**
	 * Track an object as a new parent.
	 *
	 * @param obj Identifier of the object.
	 */
	void add_parent(const fqon_t &obj);

	/**
	 * Retrieve the list of new parents.
	 *
	 * @return The list of new parents.
	 */
	const std::vector<fqon_t> &get_new_parents() const;

	/**
	 * Check if the parents were updated by the change.
	 *
	 * @return true if size of new_parents is > 1, else false.
	 */
	bool parents_update_required() const;

protected:
	/**
	 * List of new parents.
	 */
	std::vector<fqon_t> new_parents;
};


/**
 * Collects what changes have been done in a transaction.
 * Then this info is used to invalidate caches.
 */
class ChangeTracker {
public:
	/**
	 * Get the ObjectChanges for an object targeted by a patch
	 * from the changes map or create a new one if there doesn't
	 * exist one yet.
	 *
	 * @param target_name Identifier of the target object.
	 *
	 * @return An ObjectChanges tracker.
	 */
	ObjectChanges &track_patch(const fqon_t &target_name);

	/**
	 * Retrieve the map with all ObjectChanges by object.
	 *
	 * @return A map of ObjectChanges by object.
	 */
	const std::unordered_map<fqon_t, ObjectChanges> &get_object_changes() const;

	/**
	 * Get all objects whose ObjectChanges are tracked in this tracker.
	 *
	 * @return A set of object identifiers.
	 */
	std::unordered_set<fqon_t> get_changed_objects() const;

protected:
	/**
	 * Map of ObjectChanges by object.
	 */
	std::unordered_map<fqon_t, ObjectChanges> changes;
};

} // namespace nyan
