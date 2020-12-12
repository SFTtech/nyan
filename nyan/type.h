// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <set>
#include <string>
#include <vector>

#include "basic_type.h"
#include "config.h"
#include "error.h"
#include "ops.h"


namespace nyan {

class ASTMemberType;
class IDToken;
class Location;
class MetaInfo;
class Namespace;
class NamespaceFinder;
class Object;
class State;
class Token;


/**
 * Type handling for nyan values.
 */
class Type {
public:

	/**
	 * Construct type from the AST.
	 *
	 * The definition happens in a rechable name scope,
	 * where the type object is done in a specific namespace.
	 *
	 * The target name is searched in the type info database.
	 */
	Type(const ASTMemberType &ast_type,
	     const NamespaceFinder &scope,
	     const Namespace &ns,
	     const MetaInfo &type_info);

	/**
	 * Called when a composite type is created from AST.
	 */
	Type(const IDToken &token,
	     const NamespaceFinder &scope,
	     const Namespace &ns,
	     const MetaInfo &type_info);

public:

	virtual ~Type() = default;

	/**
	 * Check if this type is an object.
	 *
	 * @return true if the basic type is an object, else false.
	 */
	bool is_object() const;

	/**
	 * Check if this type is fundamental (simple non-pointer value).
	 *
	 * @return true if the basic type is fundamental, else false.
	 */
	bool is_fundamental() const;

	/**
	 * Check if this type is a composite of multiple types.
	 *
	 * @return true if the basic type is a composite, else false.
	 */
	bool is_composite() const;

	/**
	 * Check if this type is a container that stores multiple values.
	 *
	 * @return true if the basic type is a container, else false.
	 */
	bool is_container() const;

	/**
	 * Check if this type is a container of a given type.
	 *
	 * @param type Composite type that is compared to this type's basic type.
	 *
	 * @return true if the composite types matches, else false.
	 */
	bool is_container(composite_t type) const;

	/**
	 * Check if this type is a modifier.
	 *
	 * @return true if the basic type is a modifier, else false.
	 */
	bool is_modifier() const;

	/**
	 * Check if this type is a modifier of a given type.
	 *
	 * @param type Composite type that is compared to this type's basic type.
	 *
	 * @return true if the composite types matches, else false.
	 */
	bool is_modifier(composite_t type) const;

	/**
	 * Check if a value of this type is hashable.
	 *
	 * @return true if values are hashable, else false.
	 */
	bool is_hashable() const;

	/**
	 * Check if the basic type matches the given basic type, i.e. it's the same.
	 *
	 * @param type Basic type that is compared to this type's basic type.
	 *
	 * @return true if the basic types match, else false.
	 */
	bool is_basic_type_match(const BasicType &type) const;

	/**
	 * Get the object fqon of the type.
	 *
	 * @return Identifier of the object if this type is an object, else nullptr.
	 */
	const fqon_t &get_fqon() const;

	/**
	 * Get the basic type of this type, namely the primitive and composite type.
	 *
	 * @return Basic type of this type.
	 */
	const BasicType &get_basic_type() const;

	/**
	 * Get the composite type of this type.
	 *
	 * @return Composite type of this type.
	 */
	const composite_t &get_composite_type() const;

	/**
	 * Get the primitive type of this type.
	 *
	 * @return Primitive type of this type.
	 */
	const primitive_t &get_primitive_type() const;

	/**
	 * Get the composite element type of this type, i.e. the inner type
	 * that specifies the type of each item in a value.
	 *
	 * @return Pointer to the list with the element types of this type.
	 */
	const std::vector<Type> *get_element_type() const;

	/**
	 * Get the string representation of this type.
	 *
	 * @return String representation of this type.
	 */
	std::string str() const;

	/**
	 * Checks if two types are equal. Their basic type, element types
	 * and fqon must match.
	 *
	 * @param other Type that is compared with this type.
	 *
	 * @return true if the types are equal, else false.
	 */
	bool operator ==(const Type &other) const;

protected:
	/**
	 * The basic type of this Type.
	 * Stores the primitive type and the composite type.
	 */
	BasicType basic_type;

	/**
	 * If this type is a composite, the element type is stored here.
	 */
	std::shared_ptr<std::vector<Type>> element_type;

	/**
	 * If this type is an object, store the reference here.
	 * If it is nullptr, any object is covered by this type.
	 */
	fqon_t obj_ref;
};

} // namespace nyan
