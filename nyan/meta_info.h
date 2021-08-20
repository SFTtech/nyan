// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
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

	/**
	 * Add metadata information for an object.
	 *
	 * @param name Identifier of the object.
	 * @param obj_info ObjectInfo with metadata information.
	 *
	 * @return The stored metadata information object.
	 */
	ObjectInfo &add_object(const fqon_t &name, ObjectInfo &&obj_info);

	/**
	 * Get the all metadata information objects for objects
	 * stored in the database.
	 *
	 * @return Map of metadata information objects by object identifier.
	 */
	const obj_info_t &get_objects() const;

	/**
	 * Get the the metadata information object for an object.
	 *
	 * @param name Identifier of the object.
	 *
	 * @return ObjectInfo with metadata information if the object is
	 *     in the database, else nullptr.
	 */
	ObjectInfo *get_object(const fqon_t &name);

	/**
	 * Get the the metadata information object for an object.
	 *
	 * @param name Identifier of the object.
	 *
	 * @return ObjectInfo with metadata information if the object is
	 *     in the database, else nullptr.
	 */
	const ObjectInfo *get_object(const fqon_t &name) const;

	/**
	 * Check if an object is in the database.
	 *
	 * @param name Identifier of the object.
	 *
	 * @return true if the object is in the database, else false.
	 */
	bool has_object(const fqon_t &name) const;

	/**
	 * Get a string representation of all metadata information objects.
	 *
	 * @return String representation of all metadata information objects.
	 */
	std::string str() const;

protected:
	/**
	 * Location and type information for the objects.
	 * This is for displaying error messages and line information.
	 */
	obj_info_t object_info;
};

} // namespace nyan
