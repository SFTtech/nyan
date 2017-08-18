// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <unordered_map>
#include <vector>

#include "config.h"


namespace nyan {


/**
 * Change tracking for a single object.
 */
class ObjectChanges {
public:
	void add_parent(const fqon_t &obj);

	const std::vector<fqon_t> &get_new_parents() const;
	bool parents_update_required() const;

protected:
	std::vector<fqon_t> new_parents;
};


/**
 * Collects what changes have been done in a transaction.
 * Then this info is used to invalidate caches.
 */
class ChangeTracker {
public:
	ObjectChanges &track_patch(const fqon_t &name);

	const std::unordered_map<fqon_t, ObjectChanges> &get_object_changes() const;

protected:
	std::unordered_map<fqon_t, ObjectChanges> changes;
};

} // namespace nyan
