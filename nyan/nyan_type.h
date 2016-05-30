// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_TYPE_H_
#define NYAN_NYAN_TYPE_H_

#include <memory>
#include <string>

#include "nyan_error.h"
#include "nyan_ops.h"

namespace nyan {


class NyanASTMemberType;
class NyanLocation;
class NyanObject;
class NyanStore;
class NyanToken;


/**
 * Thrown when encountering type problems.
 */
class TypeError : public NyanFileError {
public:
	TypeError(const NyanLocation &location, const std::string &msg);

	virtual ~TypeError() = default;
};


/**
 * Member types available in nyan.
 * These are the primitive types.
 * A CONTAINER packs multiple primitive values together.
 * The OBJECT type requires a payload as "target" name.
 */
enum class nyan_type {
	TEXT,
	FILENAME,
	INT,
	FLOAT,
	CONTAINER,
	OBJECT
};


/**
 * Available member container types.
 */
enum class nyan_container_type {
	SINGLE,
	SET,
	ORDEREDSET,
};


/**
 * Return whether the given type is primitive.
 * Primitive types are int, float, text, etc.
 */
bool type_is_primitive(nyan_type type);

/**
 * Test if the given value token indicates a valid nyan_type,
 * then return it.
 * A value token is e.g. "1337", "'rofl'" or "SomeThing".
 * throws ASTError if it fails.
 */
nyan_type type_from_value_token(const NyanToken &token);

/**
 * Test if the given type token declares a valid nyan_type,
 * returns it. Also returns the container type.
 * A type token is e.g. "int" or "float" or "SomeObject".
 * If it is e.g. "set", type will be CONTAINER and the container type SET.
 * throws ASTError if it fails.
 */
std::pair<nyan_type, nyan_container_type>
type_from_type_token(const NyanToken &token);


/**
 * Get a string representation of a basic nyan type.
 */
constexpr const char *type_to_string(nyan_type type) {
	switch (type) {
	case nyan_type::TEXT:          return "text";
	case nyan_type::FILENAME:      return "file";
	case nyan_type::INT:           return "int";
	case nyan_type::FLOAT:         return "float";
	case nyan_type::CONTAINER:     return "container";
	case nyan_type::OBJECT:        return "object";
	}
	return "unhandled nyan_type";
}


/**
 * Type handling for nyan values.
 */
class NyanType {
public:
	/**
	 * Construct from the AST.
	 * For type object target lookups, require the store.
	 */
	NyanType(const NyanASTMemberType &ast_type,
	         const NyanStore &store);

	/**
	 * Use an object as a type.
	 * This creates a nyan type that matches the given object
	 * or any child of it.
	 * nullptr means it can match any object.
	 */
	NyanType(NyanObject *target);

	/**
	 * Create a container type.
	 * Container value type must be provided.
	 */
	NyanType(nyan_container_type container_type,
	         std::unique_ptr<NyanType> &&value_type);

	/**
	 * Construct a type from a parser token.
	 * Used to construct a type for a value specification
	 * or from a type declaration.
	 *
	 * As this can look up nyanobjects,
	 * the object store is required.
	 *
	 * @param is_type_decl specifies whether the token denotes a type
	 *                     declaration or a value.
	 */
	NyanType(const NyanToken &token,
	         const NyanStore &store,
	         bool is_type_decl);

	// move to other type
	NyanType(NyanType &&other);
	NyanType &operator =(NyanType &&other);

	// no copies, use the NyanTypeContainer for that.
	NyanType(const NyanType &other) = delete;
	NyanType &operator =(const NyanType &other) = delete;

	virtual ~NyanType() = default;

	/**
	 * Return if this type is primitive (simple non-pointer value).
	 */
	bool is_primitive() const;

	/**
	 * Return if this type is a container that stores multiple values.
	 */
	bool is_container() const;

	/**
	 * Test if is a container of the given type.
	 */
	bool is_container(nyan_container_type type) const;

	/**
	 * Check if this type is a child of another type.
	 * This also verifies container compatibility.
	 */
	bool is_child_of(const NyanType &other) const;

	/**
	 * Check if this type can be in the given other type.
	 * This will of course only suceed if other is a container.
	 */
	bool can_be_in(const NyanType &other) const;

	/**
	 * Return the container variant of this type.
	 */
	nyan_container_type get_container_type() const;

	/**
	 * Return the basic type of this NyanType.
	 */
	nyan_type get_type() const;

	/**
	 * Return a string representation of this type.
	 */
	std::string str() const;

protected:
	/**
	 * Basic type.
	 * Decides if this NyanType is primitive, a container, or an object.
	 */
	nyan_type type;

	/**
	 * Stores if this type is a container and if yes, which one.
	 */
	nyan_container_type container_type;

	/**
	 * If this type is a container, the value type is stored here.
	 */
	std::unique_ptr<NyanType> value_type;

	/**
	 * If this type is an object, store the target here.
	 * If it is nullptr, any object is covered by this type.
	 */
	NyanObject *target;
};

} // namespace nyan

#endif
