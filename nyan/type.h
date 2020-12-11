// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
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
	 * Return if this type is an object.
	 */
	bool is_object() const;

	/**
	 * Return if this type is fundamental (simple non-pointer value).
	 */
	bool is_fundamental() const;

	/**
	 * Return if this type is a composite of multiple types.
	 */
	bool is_composite() const;

	/**
	 * Return if this type is a container that stores multiple values.
	 */
	bool is_container() const;

	/**
	 * Test if this is a container of the given type.
	 */
	bool is_container(composite_t type) const;

	/**
	 * Return if this type is a modifier.
	 */
	bool is_modifier() const;

	/**
	 * Test if is a modifier of the given type.
	 */
	bool is_modifier(composite_t type) const;

	/**
	 * Return if a value of this type is hashable.
	 */
	bool is_hashable() const;

	/**
	 * Test if the basic type matches the given type, i. e. it's the same.
	 */
	bool is_basic_type_match(const BasicType &type) const;

	/**
	 * Check if this type can contain the given other type.
	 * This will only succeed if this type is a composite.
	 *
	 * @param meta_info Meta-information of the database. Used for finding object linearization.
	 * @param other Contained type candidate.
	 * @param pos Position of the element type in this type that other is compared with.
	 */
	bool can_contain(const MetaInfo &meta_info,
	                 const Type &other,
	                 const unsigned int pos = 0,
	                 std::set<composite_t> modflags = {}) const;

	/**
	 * Check if this type is compatible to the given other type.
	 * This will only succeed if both types are objects.
	 *
	 * @param meta_info Meta-information of the database. Used for finding object linearization.
	 * @param other fqon of the object
	 */
	bool can_hold(const MetaInfo &meta_info,
	              const fqon_t &other,
	              std::set<composite_t> modflags = {}) const;

	/**
	 * Return the object fqon.
	 */
	const fqon_t &get_fqon() const;

	/**
	 * Return the basic type, namely the primitive and composite type.
	 */
	const BasicType &get_basic_type() const;

	/**
	 * Return the composite type of this type.
	 */
	const composite_t &get_composite_type() const;

	/**
	 * Return the primitive type of this Type.
	 */
	const primitive_t &get_primitive_type() const;

	/**
	 * Get the composite element type, i. e. the inner type
	 * that specifies the type of each element.
	 */
	const std::vector<Type> *get_element_type() const;

	/**
	 * Return a string representation of this type.
	 */
	std::string str() const;

	/**
	 * Checks if two types are the same.
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
