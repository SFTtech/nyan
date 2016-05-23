// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_STORE_H_
#define NYAN_NYAN_STORE_H_

#include <memory>
#include <string>

#include "nyan_namespace.h"


namespace nyan {

/**
 * Nyan data storage space.
 */
class NyanStore {
	friend class NyanObject;
	friend class NyanNamespace;

public:
	NyanStore();
	virtual ~NyanStore() = default;

	/**
	 * Add the given nyan object to the store.
	 * returns a pointer to the storage position.
	 */
	NyanObject *add(std::unique_ptr<NyanObject> &&obj);

	/**
	 * Return the NyanObject with given name.
	 * returns nullptr if not found.
	 */
	NyanObject *get(const std::string &name) const;

protected:
	// TODO: caching for:
	//       * namespaces
	//       * nyan object namespace paths

	/**
	 * The root namespace
	 */
	NyanNamespace root;
};

} // namespace nyan

#endif
