// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_DATABASE_H_
#define NYAN_NYAN_DATABASE_H_

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
class NyanDatabase {
	friend class NyanObject;
	friend class NyanNamespace;

public:
	NyanDatabase(NyanDatabase *parent=nullptr);
	virtual ~NyanDatabase();

	/**
	 * Add the given nyan object to the store.
	 * returns a pointer to the storage position.
	 */
	NyanObject *add(std::unique_ptr<NyanObject> &&obj);

	/**
	 * Return the NyanObject with given full name.
	 * returns nullptr if not found.
	 */
	NyanObject *get(const std::string &name) const;

protected:
	/**
	 * The parent database which this database overlays.
	 */
	NyanDatabase *parent;

	/**
	 * The root namespace.
	 */
	NyanNamespace root;

	/**
	 * Object unique name -> Object map.
	 * The unique name is the namespace.objectname
	 * e.g. "government.nsa.quantuminsert.timings"
	 */
	std::unordered_map<std::string, std::unique_ptr<NyanObject>> objects;
};

} // namespace nyan

#endif
