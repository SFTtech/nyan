// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
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
	NONE,
	CONTAINER,
	MODIFIER,
};


/**
 * Available member composite types (containers or modifiers).
 * SINGLE means it's not a composite.
 */
enum class composite_t {
	// primitive value
	SINGLE,

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
	 *
	 * @return true if the basic type is an object, else false.
	 */
	bool is_object() const;


	/**
	 * Return whether the given type is fundamental.
	 * Primitive types are int, float, text, etc.
	 * Non-primitive types are objects, containers and modifiers.
	 *
	 * @return true if the basic type is fundamental, else false.
	 */
	bool is_fundamental() const;


	/**
	 * Test if this basic type is a composite.
	 * that is, the composite type is not SINGLE.
	 *
	 * @return true if the basic type is a composite, else false.
	 */
	bool is_composite() const;


	/**
	 * Test if this basic type is a container.
	 * that is, the composite type is one of the container
	 * types SET, ORDEREDSET or DICT.
	 *
	 * @return true if the basic type is a container, else false.
	 */
	bool is_container() const;


	/**
	 * Test if this basic type is a modifier.
	 * that is, the composite type is one of the modifier
	 * types ABSTRACT, CHILDREN or OPTIONAL.
	 *
	 * @return true if the basic type is a modifier, else false.
	 */
	bool is_modifier() const;


	/**
	 * Equality comparison.
	 *
	 * @return true if the basic types are exactly the same, else false.
	 */
	bool operator==(const BasicType &other) const;


	/**
	 * Test if the given type token declares a valid primitive
	 * and composite type.
	 * A type token is e.g. "int" or "float" or "SomeObject".
	 * If it is e.g. "set", type will be CONTAINER and the composite type SET.
	 *
	 * @param token An IDToken that contains a type identifier or a nyan object name.
	 *
	 * @return BasicType declared by the token.
	 * @throw ASTError if no typename could be found.
	 */
	static BasicType from_type_token(const IDToken &token);


	/**
	 * Returns how many nested types are required when declaring a given
	 * type.
	 *
	 * @param type The tested type.
	 *
	 * @return Number of required nested types.
	 */
	static unsigned int expected_nested_types(const BasicType &type) {
		if (type.is_fundamental()) {
			return 0;
		}

		switch (type.composite_type) {
			// containers
		case composite_t::SET:
			return 1;
		case composite_t::ORDEREDSET:
			return 1;
		case composite_t::DICT:
			return 2;

			// modifiers
		case composite_t::ABSTRACT:
			return 1;
		case composite_t::CHILDREN:
			return 1;
		case composite_t::OPTIONAL:
			return 1;

			// else, must be an object
		default:
			return 0;
		}
	}
};


/**
 * Get a string representation of a basic nyan type.
 *
 * @param type A primitive type.
 *
 * @return String representation of the type.
 */
constexpr const char *type_to_string(primitive_t type) {
	switch (type) {
	case primitive_t::BOOLEAN:
		return "bool";
	case primitive_t::TEXT:
		return "text";
	case primitive_t::FILENAME:
		return "file";
	case primitive_t::INT:
		return "int";
	case primitive_t::FLOAT:
		return "float";
	case primitive_t::OBJECT:
		return "object";
	case primitive_t::NONE:
		return "none";
	case primitive_t::CONTAINER:
		return "container";
	case primitive_t::MODIFIER:
		return "modifier";
	}

	return "unhandled primitive_t";
}


/**
 * Get a string represenation for a nyan composite type.
 *
 * @param type A composite type.
 *
 * @return String representation of the type.
 */
constexpr const char *composite_type_to_string(composite_t type) {
	switch (type) {
	case composite_t::SINGLE:
		return "single_value";
	case composite_t::SET:
		return "set";
	case composite_t::ORDEREDSET:
		return "orderedset";
	case composite_t::DICT:
		return "dict";
	case composite_t::ABSTRACT:
		return "abstract";
	case composite_t::CHILDREN:
		return "children";
	case composite_t::OPTIONAL:
		return "optional";
	}

	return "unhandled composite_t";
}

} // namespace nyan
