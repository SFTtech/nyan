// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <deque>
#include <memory>
#include <string>

#include "member.h"


namespace nyan {

class ObjectChanges;
class ObjectInfo;


/**
 * Single object state storage.
 */
class ObjectState {
	friend class Database;
public:
	/**
	 * Creation of an initial object state.
	 */
	ObjectState(std::deque<fqon_t> &&parents);

	/**
	 * Patch application.
	 */
	void apply(const std::shared_ptr<ObjectState> &other,
	           const ObjectInfo &mod_info,
	           ObjectChanges &tracker);

	std::shared_ptr<ObjectState> copy() const;

	const std::deque<fqon_t> &get_parents() const;

	bool has(const memberid_t &name) const;
	Member *get(const memberid_t &name);
	const Member *get(const memberid_t &name) const;
	const std::unordered_map<memberid_t, Member> &get_members() const;

	std::string str() const;

private:
	/**
	 * Replace the member map.
	 * Used for creating initial object states.
	 */
	void set_members(std::unordered_map<memberid_t, Member> &&members);

	/**
	 * Parent objects.
	 */
	std::deque<fqon_t> parents;

	/**
	 * Member storage.
	 */
	std::unordered_map<memberid_t, Member> members;

	// The object location is stored in the metainfo-database.
};

} // namespace nyan
