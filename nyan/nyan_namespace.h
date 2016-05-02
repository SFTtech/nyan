// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_NAMESPACE_H_
#define NYAN_NYAN_NAMESPACE_H_

#include <memory>
#include <unordered_map>

#include "nyan_object.h"

namespace nyan {

/**
 * One namespace in the nyan tree.
 * Contains many nyanobjects.
 */
class NyanNamespace {
	friend class NyanObject;

public:
	NyanNamespace();
	virtual ~NyanNamespace() = default;

	NyanObject *add_obj(std::unique_ptr<NyanObject> &&obj);
	void add_namespace(const std::string &name);

	NyanObject *get_obj(const std::string &name) const;
	NyanNamespace *get_namespace(const std::string &name) const;

protected:
	/**
	 * Object name -> Object map.
	 */
	std::unordered_map<std::string, std::unique_ptr<NyanObject>> objects;

	/**
	 * Subnamespaces of this namespace.
	 */
	std::unordered_map<std::string, std::unique_ptr<NyanNamespace>> namespaces;
};

} // namespace nyan

#endif
