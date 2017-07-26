// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <memory>

#include "member.h"


namespace nyan {

/**
 * Single object state storage.
 */
class ObjectState {
	friend class Database;
public:
	/**
	 * Creation of an initial object state.
	 */
	ObjectState(std::vector<fqon_t> &&parents);

	/**
	 * Patch application.
	 */
	void apply(const std::shared_ptr<ObjectState> &other);

	std::shared_ptr<ObjectState> copy() const;

	const std::vector<fqon_t> &get_parents() const;

	bool has_member(const memberid_t &name) const;
	const Member *get_member(const memberid_t &name) const;
	const std::unordered_map<memberid_t, Member> &get_members() const;

	void set_linearization(std::vector<fqon_t> &&lin);
	const std::vector<fqon_t> &get_linearization() const;

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
	std::vector<fqon_t> parents;

	/**
	 * Member storage.
	 */
	std::unordered_map<memberid_t, Member> members;

	/**
	 * Linearization of parent objects.
	 * TODO asdf: when is this invalidated?
	 */
	std::vector<fqon_t> linearization;

	/**
	 * Tracking of childs of this object.
	 * Used for event-based triggering,
	 * i.e. firing callbacks when a member value changed.
	 * TODO: implement object child tracking :)
	 */
	std::vector<fqon_t> children;

	// The object location is stored in the metainfo-database.
};

} // namespace nyan
