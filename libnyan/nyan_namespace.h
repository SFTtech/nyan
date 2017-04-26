// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_NAMESPACE_H_
#define NYAN_NYAN_NAMESPACE_H_

#include <memory>
#include <unordered_map>

namespace nyan {

class NyanDatabase;
class NyanObject;


/**
 * One namespace in the nyan tree.
 * Contains many nyanobjects.
 */
class NyanNamespace {

public:
	NyanNamespace();
	virtual ~NyanNamespace() = default;

	NyanNamespace *add_namespace(const std::string &name);

	NyanObject *get_obj(const std::string &name) const;
	NyanNamespace *get_namespace(const std::string &name) const;

protected:

	/**
	 * Database this namespace is in.
	 */
	NyanDatabase *database;
};

} // namespace nyan

#endif
