// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "config.h"
#include "object_info.h"


namespace nyan {

/**
 * Nyan database metainformation.
 * Used for type-checking etc.
 */
class MetaInfo {
public:
	using obj_info_t = std::unordered_map<fqon_t, ObjectInfo>;

	MetaInfo() = default;
	~MetaInfo() = default;

	ObjectInfo &add_object(const fqon_t &name, ObjectInfo &&obj);

	const obj_info_t &get_objects() const;

	ObjectInfo *get_object(const fqon_t &name);

	bool has_object(const fqon_t &name) const;

	std::string str() const;

protected:
	/**
	 * Location and type information for the objects.
	 * This is for displaying error messages and line information.
	 */
	obj_info_t object_info;
};

} // namespace nyan
