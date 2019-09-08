// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "change_tracker.h"


namespace nyan {

void ObjectChanges::add_parent(const fqon_t &obj) {
	this->new_parents.push_back(obj);
}


const std::vector<fqon_t> &ObjectChanges::get_new_parents() const {
	return this->new_parents;
}


bool ObjectChanges::parents_update_required() const {
	return this->new_parents.size() > 0;
}


ObjectChanges &ChangeTracker::track_patch(const fqon_t &target_name) {
	// if existing, return the object change tracker
	// else: create a new one.
	auto it = this->changes.find(target_name);
	if (it == std::end(this->changes)) {
		return this->changes.emplace(
			target_name,
			ObjectChanges{}
		).first->second;
	}
	else {
		return it->second;
	}
}


const std::unordered_map<fqon_t, ObjectChanges> &ChangeTracker::get_object_changes() const {
	return this->changes;
}


std::unordered_set<fqon_t> ChangeTracker::get_changed_objects() const {
	std::unordered_set<fqon_t> ret;
	ret.reserve(this->changes.size());

	for (auto &it : this->changes) {
		ret.insert(it.first);
	}

	return ret;
}


} // namespace nyan
