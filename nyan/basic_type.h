// Copyright 2016-2018 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "config.h"


namespace nyan {

class IDToken;


/**
 * Member types available in nyan.
 * These are the primitive types.
 * A CONTAINER packs multiple primitive values together.
 * The OBJECT type requires a payload as "target" name.
 */
enum class primitive_t {
	BOOLEAN,
	TEXT,
	FILENAME,
	INT,
	FLOAT,
	CONTAINER,
	OBJECT
};


/**
 * Available member container types.
 * Single means it's not a container.
 */
enum class container_t {
	SINGLE,
	SET,
	ORDEREDSET,
};


/**
 * Basic nyan type information.
 * Stores a combination of the primitive type
 * and the container type.
 *
 * This is for storing and handling the built-in types.
 * Custom types are then added with the `Type` class.
 */
class BasicType {
public:
	/**
	 * Primitive type.
	 * Decides if this Type is primitive, a container, or an object.
	 */
	primitive_t primitive_type;

	/**
	 * Stores if this type is a container and if yes, which one.
	 */
	container_t container_type;

	/**
	 * Return whether the type is object.
	 */
	bool is_object() const;

	/**
	 * Return whether the given type is fundamental.
	 * Primitive types are int, float, text, etc.
	 * Non-primitive types are containers and objects.
	 */
	bool is_fundamental() const;


	/**
	 * Test if this basic type is a container,
	 * that is, the container type is not SINGLE
	 * or primitive type is CONTAINER.
	 */
	bool is_container() const;


	/**
	 * Equality comparison.
	 */
	bool operator ==(const BasicType &other) const;


	/**
	 * Test if the given type token declares a valid primitive_t,
	 * returns it. Also returns the container type.
	 * A type token is e.g. "int" or "float" or "SomeObject".
	 * If it is e.g. "set", type will be CONTAINER and the container type SET.
	 * throws ASTError if it fails.
	 */
	static BasicType from_type_token(const IDToken &token);
};


/**
 * Get a string representation of a basic nyan type.
 */
constexpr const char *type_to_string(primitive_t type) {
	switch (type) {
	case primitive_t::BOOLEAN:       return "bool";
	case primitive_t::TEXT:          return "text";
	case primitive_t::FILENAME:      return "file";
	case primitive_t::INT:           return "int";
	case primitive_t::FLOAT:         return "float";
	case primitive_t::CONTAINER:     return "container";
	case primitive_t::OBJECT:        return "object";
	}

	return "unhandled primitive_t";
}


/**
 * Get a string represenation for a nyan container type.
 */
constexpr const char *container_type_to_string(container_t type) {
	switch (type) {
	case container_t::SINGLE:        return "single_value";
	case container_t::SET:           return "set";
	case container_t::ORDEREDSET:    return "orderedset";
	}

	return "unhandled container_t";
}

} // namespace nyan
