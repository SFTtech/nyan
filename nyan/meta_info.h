// Copyright 2017-2023 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "config.h"
#include "object_info.h"
#include "namespace.h"


namespace nyan {

/**
 * Nyan database metainformation.
 * Used for type-checking etc.
 */
class MetaInfo {
public:
	using obj_info_t = std::unordered_map<fqon_t, ObjectInfo>;
	using ns_info_t = std::unordered_map<fqon_t, Namespace>;

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
	 * Add a namespace to the database.
	 *
	 * @param ns Namespace to add.
	 *
	 * @return The stored namespace.
	 */
	Namespace &add_namespace(const Namespace &ns);

	/**
	 * Get a namespace from the database.
	 *
	 * @param name Identifier of the namespace.
	 *
	 * @return The stored namespace.
	 */
	Namespace *get_namespace(const fqon_t &name);

	/**
	 * Get a namespace from the database.
	 *
	 * @param name Identifier of the namespace.
	 *
	 * @return The stored namespace.
	 */
	const Namespace *get_namespace(const fqon_t &name) const;

	/**
	 * Check if a namespace is in the database.
	 *
	 * @param name Identifier of the namespace.
	 *
	 * @return true if the namespace is in the database, else false.
	 */
	bool has_namespace(const fqon_t &name) const;

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

	/**
	 * Namespaces loaded in the database.
	 */
	ns_info_t namespaces;
};

} // namespace nyan
