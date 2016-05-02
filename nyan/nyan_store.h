// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_STORE_H_
#define NYAN_NYAN_STORE_H_


#include <string>
#include <unordered_map>
#include <vector>

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

	NyanObject *add(std::unique_ptr<NyanObject> &&obj);
	NyanObject *get(const std::string &name) const;

protected:
	// TODO: caching for:
	//       * values
	//       * namespaces
	//       * nyan object namespace paths

	/**
	 * The root namespace
	 */
	NyanNamespace root;

	/**
	 * Cache for the linearization calculation.
	 */
	std::unordered_map<NyanObject *, std::vector<NyanObject *>> linearizations;
};


} // namespace nyan

#endif
