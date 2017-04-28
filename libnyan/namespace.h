// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_NAMESPACE_H_
#define NYAN_NYAN_NAMESPACE_H_

#include <memory>
#include <unordered_map>

namespace nyan {

class Database;
class Object;


/**
 * One namespace in the nyan tree.
 * Contains many nyanobjects.
 */
class Namespace {

public:
	Namespace();
	virtual ~Namespace() = default;

	Namespace *add_namespace(const std::string &name);

	Object *get_obj(const std::string &name) const;
	Namespace *get_namespace(const std::string &name) const;

protected:

	/**
	 * Database this namespace is in.
	 */
	Database *database;
};

} // namespace nyan

#endif
