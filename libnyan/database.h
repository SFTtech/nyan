// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <string>
#include <unordered_map>

#include "namespace.h"


namespace nyan {

/**
 * Nyan database.
 * This is the main entry handle for accessing and modifying data
 * stored in nyan.
 */
class Database {
	friend class Object;
	friend class Namespace;

public:
	Database(Database *parent=nullptr);
	virtual ~Database();

	/**
	 * Add the given nyan object to the store.
	 * returns a pointer to the storage position.
	 */
	Object *add(std::unique_ptr<Object> &&obj);

	/**
	 * Return the Object with given full name.
	 * returns nullptr if not found.
	 */
	Object *get(const std::string &name) const;

protected:
	/**
	 * The parent database which this database overlays.
	 */
	Database *parent;

	/**
	 * The root namespace.
	 */
	Namespace root;

	/**
	 * Object unique name -> Object map.
	 * The unique name is the namespace.objectname
	 * e.g. "government.nsa.quantuminsert.timings"
	 */
	std::unordered_map<std::string, std::unique_ptr<Object>> objects;
};

} // namespace nyan
