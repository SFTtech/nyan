// Copyright 2016-2023 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <vector>

#include "config.h"


namespace nyan {

class IDToken;

/**
 * Identifier of a namespace, i.e. a directory, filename, or (nested) object.
 */
class Namespace {
	friend struct std::hash<Namespace>;

public:
	/**
	 * Create a namespace identifier.
	 *
	 * @param dir_components Directory path components.
	 * @param filename Filename component.
	 * @param obj_components Object components.
	 */
	Namespace(std::vector<std::string> &&dir_components = {},
	          std::string &&filename = "",
	          std::vector<std::string> &&obj_components = {});
	Namespace(const std::vector<std::string> &dir_components = {},
	          const std::string &filename = "",
	          const std::vector<std::string> &obj_components = {});
	Namespace(const Namespace &other, const std::string &obj_addend);

	~Namespace() = default;

	/**
	 * Pop the last component from the namespace ID.
	 */
	void pop_last();

	/**
	 * Check if the namespace ID is empty.
	 *
	 * @return true if the namespace ID is empty, else false.
	 */
	bool empty() const;

	/**
	 * Append the given object name to the namespace identifier and get
	 * the resulting identifier
	 *
	 * @param name IDToken with an object reference.
	 * @param skip Number of components at the start of \p name to be skipped.
	 *
	 * @return fqon of the object.
	 */
	fqon_t combine(const IDToken &name, size_t skip = 0) const;

	/**
	 * Check if the namespace ID refers to a directory.
	 *
	 * @return true if the namespace ID is a directory, else false.
	 */
	bool is_dir() const;

	/**
	 * Check if the namespace ID refers to a file.
	 *
	 * @return true if the namespace ID is a file, else false.
	 */
	bool is_file() const;

	/**
	 * Check if the namespace ID refers to a nyan object.
	 *
	 * @return true if the namespace ID is an object, else false.
	 */
	bool is_obj() const;

	/**
	 * Check if the namespace ID refers to a nested nyan object.
	 *
	 * @return true if the namespace ID is a nested object, else false.
	 */
	bool is_nested_obj() const;

	/**
	 * Get the directory path components of the namespace ID.
	 *
	 * @return Directory path components.
	 */
	const std::vector<std::string> &get_dir_components() const;

	/**
	 * Get the filename component of the namespace ID.
	 *
	 * @return Filename component.
	 */
	const std::string &get_filename() const;

	/**
	 * Get the object components of the namespace ID.
	 *
	 * @return Object components.
	 */
	const std::vector<std::string> &get_obj_components() const;

	/**
	 * Get the directory path from the directory components inside the
	 * namespace.
	 *
	 * @return Directory path from the namespace.
	 */
	std::string to_dirpath() const;

	/**
	 * Get the file path from the directory and filename components inside
	 * the namespace.
	 *
	 * If the namespace ID refers to an object, this is the path to the file
	 * containing the object.
	 *
	 * @return File path from the namespace.
	 */
	std::string to_filepath() const;

	/**
	 * Get the fqon for this namespace ID.
	 *
	 * @return fqon identifier.
	 */
	fqon_t to_fqon() const;

	/**
	 * Get a string representation of this namespace.
	 *
	 * @return String representation of this namespace.
	 */
	std::string str() const;

	/**
	 * Checks if this namespace is equal to a given namespace.
	 *
	 * @return true if the namespaces are equal, else false.
	 */
	bool operator==(const Namespace &other) const;

	/**
	 * Create a namespace from a given filename. Performs a sanity
	 * check on the filename.
	 *
	 * @param filename Name of a file, including the extension.
	 *
	 * @return Namespace for the filename.
	 */
	static Namespace from_filename(const std::string &filename);

private:
	/**
	 * Directory path components.
	 */
	std::vector<std::string> dir_components;

	/**
	 * Filename component.
	 */
	std::string filename;

	/**
	 * Object components.
	 */
	std::vector<std::string> obj_components;
};

} // namespace nyan


namespace std {
template <>
struct hash<nyan::Namespace> {
	size_t operator()(const nyan::Namespace &ns) const;
};
} // namespace std
