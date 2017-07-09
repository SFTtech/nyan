// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <unordered_set>

#include "../ops.h"
#include "../type.h"
#include "value_holder.h"


namespace nyan {

class ASTMemberValue;
class Object;
class Member;
class Namespace;
class NamespaceFinder;


/**
 * Base class for all possible member values.
 */
class Value {
public:
	Value();
	virtual ~Value() = default;

	/**
	 * Create a value of this type from the AST.
	 */
	static ValueHolder from_ast(const Type &target_type,
	                            const ASTMemberValue &val,
	                            const NamespaceFinder &scope,
	                            const Namespace &ns,
	                            const MetaInfo &names);

	/**
	 * Return a copy of this Value.
	 */
	virtual ValueHolder copy() const = 0;

	/**
	 * Apply the given change to this value.
	 * This value will be modified by the change-member's
	 * operation and value.
	 */
	void apply(const Member &change);

	/**
	 * String representation of the value.
	 */
	virtual std::string str() const = 0;

	/**
	 * Smaller string representation of the value.
	 */
	virtual std::string repr() const = 0;

	/**
	 * Hash function for the value.
	 */
	virtual size_t hash() const = 0;

	/**
	 * Return the basic type of this value.
	 * This means the primitive and the container type are provided,
	 * but not container element types and object targets, because
	 * types are not inferred for values.
	 */
	virtual const BasicType &get_type() const = 0;

	/**
	 * Return a set of allowed operations
	 * that are allowed for "entry_type.$operation(this)".
	 *
	 * This means the allowed operations can be used to assign/manipulate the
	 * member entry with this value.
	 */
	virtual const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const = 0;

	/**
	 * Comparison for Values.
	 * Performs the typeid comparison, then calls this->equals(other).
	 */
	bool operator ==(const Value &other) const;

	/**
	 * Inequality operator, it's just "not ==".
	 */
	bool operator !=(const Value &other) const;

protected:
	/**
	 * Value-specific comparison function.
	 * It casts the value dynamically, but no type check is performed!
	 */
	virtual bool equals(const Value &other) const = 0;

	/**
	 * Apply the given change to the value.
	 * Internally casts the value dynamically,
	 * the type check must be done before calling this function.
	 */
	virtual void apply_value(const Value &value, nyan_op operation) = 0;

};
} // namespace nyan


namespace std {

/**
 * Hash for Values.
 */
template<>
struct hash<nyan::Value *> {
	size_t operator ()(const nyan::Value *val) const {
		return val->hash();
	}
};

} // namespace std
