// Copyright 2016-2018 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "config.h"


namespace nyan {

class IDToken;


/**
 * Member types available in nyan.
 * These are the primitive types.
 * The OBJECT type requires a payload as "target" name.
 * A CONTAINER packs multiple primitive values together.
 * A MODIFIER is an extension of a contained type.
 */
enum class primitive_t {
	BOOLEAN,
	TEXT,
	FILENAME,
	INT,
	FLOAT,
	OBJECT,
	CONTAINER,
	MODIFIER,
};


/**
 * Available member composite types (containers or modifiers).
 * NONE means it's not a composite.
 */
enum class composite_t {
	// primitive value
	NONE,

	// Containers
	SET,
	ORDEREDSET,
	DICT,

	// Modifiers
	ABSTRACT,
	CHILDREN,
	OPTIONAL,
};


/**
 * Basic nyan type information.
 * Stores a combination of the primitive type
 * and the composite type.
 *
 * This is for storing and handling the built-in types.
 * Custom types are then added with the `Type` class.
 */
class BasicType {
public:
	/**
	 * Primitive type.
	 * Decides if this Type is primitive, an object, a container
	 * or a modifier.
	 */
	primitive_t primitive_type;

	/**
	 * Stores if this type is a composite if types and if yes, which one.
	 * Composite types can be containers or modifiers.
	 */
	composite_t composite_type;

	/**
	 * Return whether the type is object.
	 */
	bool is_object() const;


	/**
	 * Return whether the given type is fundamental.
	 * Primitive types are int, float, text, etc.
	 * Non-primitive types are objects, containers and modifiers.
	 */
	bool is_fundamental() const;


	/**
	 * Test if this basic type is a composite.
	 * that is, the composite type is not NONE.
	 */
	bool is_composite() const;


	/**
	 * Test if this basic type is a container.
	 * that is, the composite type is one of the container
	 * types SET, ORDEREDSET or DICT.
	 */
	bool is_container() const;


	/**
	 * Test if this basic type is a modifier.
	 * that is, the composite type is one of the modifier
	 * types ABSTRACT, CHILDREN or OPTIONAL.
	 */
	bool is_modifier() const;


	/**
	 * Equality comparison.
	 */
	bool operator ==(const BasicType &other) const;


	/**
	 * Test if the given type token declares a valid primitive_t,
	 * returns it. Also returns the composite type.
	 * A type token is e.g. "int" or "float" or "SomeObject".
	 * If it is e.g. "set", type will be CONTAINER and the composite type SET.
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
	case primitive_t::OBJECT:        return "object";
	case primitive_t::CONTAINER:     return "container";
	case primitive_t::MODIFIER:      return "modifier";
	}

	return "unhandled primitive_t";
}


/**
 * Get a string represenation for a nyan composite type.
 */
constexpr const char *composite_type_to_string(composite_t type) {
	switch (type) {
	case composite_t::NONE:          return "single_value";
	case composite_t::SET:           return "set";
	case composite_t::ORDEREDSET:    return "orderedset";
	case composite_t::DICT:          return "dict";
	case composite_t::ABSTRACT:      return "abstract";
	case composite_t::CHILDREN:      return "children";
	case composite_t::OPTIONAL:      return "optional";
	}

	return "unhandled composite_t";
}

} // namespace nyan
