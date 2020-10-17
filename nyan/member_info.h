// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>

#include "location.h"


namespace nyan {

class Type;


/**
 * Stores information for a member of an Object.
 */
class MemberInfo {
public:
	explicit MemberInfo(const Location &location);
	~MemberInfo() = default;

	Type &set_type(std::shared_ptr<Type> &&type, bool initial);
	Type &set_type(const std::shared_ptr<Type> &type, bool initial);
	const std::shared_ptr<Type> &get_type() const;

	const Location &get_location() const;

	bool is_initial_def() const;

	std::string str() const;

protected:
	/**
	 * Location where the member was defined.
	 */
	Location location;

	/**
	 * is this member definition the initial one?
	 */
	bool initial_def;

	/**
	 * Type of the member.
	 */
	std::shared_ptr<Type> type;
};


} // namespace nyan
