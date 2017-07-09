// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <unordered_map>
#include <vector>

#include "config.h"
#include "location.h"
#include "member_info.h"


namespace nyan {

/**
 * Information about an object.
 * Used for the type system and error messages.
 */
class ObjectInfo {
public:
	using member_info_t = std::unordered_map<memberid_t, MemberInfo>;

	explicit ObjectInfo(const Location &location);
	~ObjectInfo() = default;

	MemberInfo &add_member(const memberid_t &name, MemberInfo &&member);

	member_info_t &get_members();

	const MemberInfo *get_member(const memberid_t &name) const;

	void set_target(const fqon_t &name);

	void add_inheritance_add(const fqon_t &name);

	std::string str() const;

protected:
	/**
	 * Location where the object was defined.
	 */
	Location location;

	/**
	 * Patch target name.
	 */
	fqon_t target;

	/**
	 * True if this patch or a load-time parent has a target.
	 */
	bool is_patch = false;

	/**
	 * List of objects to add to the patch target.
	 */
	std::vector<fqon_t> inheritance_add;

	/**
	 * Maps members to their information.
	 */
	member_info_t member_info;
};


} // namespace nyan
