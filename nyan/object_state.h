// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
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
	 * Apply changes to this object with a patch.
	 *
	 * @param mod Shared pointer to the patch modifying this object.
	 * @param mod_info Metadata information object of the patch.
	 * @param tracker Tracker for changes to this object.
	 */
	void apply(const std::shared_ptr<ObjectState> &mod,
	           const ObjectInfo &mod_info,
	           ObjectChanges &tracker);

	/**
	 * Copy this object state.
	 *
	 * @return Shared pointer to the copy of the object state.
	 */
	std::shared_ptr<ObjectState> copy() const;

	/**
	 * Get the parents of this object.
	 *
	 * @return Double-ended queue with identifiers for the parent objects.
	 */
	const std::deque<fqon_t> &get_parents() const;

	/**
	 * Check if the object has a member with a given identifier.
	 *
	 * @param name Identifier of the member.
	 *
	 * @return true if the object has a member with the identifier, else false.
	 */
	bool has(const memberid_t &name) const;

	/**
	 * Get the pointer to a member with a given identifier.
	 *
	 * @param name Identifier of the member.
	 *
	 * @return Pointer to the member if the object has this member, else nullptr.
	 */
	Member *get(const memberid_t &name);

	/**
	 * Get the pointer to a member with a given identifier.
	 *
	 * @param name Identifier of the member.
	 *
	 * @return Pointer to the member object if the object has this member, else nullptr.
	 */
	const Member *get(const memberid_t &name) const;

	/**
	 * Get the members of this object.
	 *
	 * @return Map of the member objects by member identifer.
	 */
	const std::unordered_map<memberid_t, Member> &get_members() const;

	/**
	 * Get the string representation of this object state.
	 *
	 * @return String representation of this object state.
	 */
	std::string str() const;

private:
	/**
	 * Replace the member map. Used for creating initial object states.
	 *
	 * @param members Map of the member objects by member identifer.
	 */
	void set_members(std::unordered_map<memberid_t, Member> &&members);

	/**
	 * Parent objects.
	 */
	std::deque<fqon_t> parents;

	/**
	 * Member objects storage.
	 */
	std::unordered_map<memberid_t, Member> members;

	// The object location is stored in the metainfo-database.
};

} // namespace nyan
