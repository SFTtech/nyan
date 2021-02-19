// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <unordered_set>

#include "../ops.h"
#include "../type.h"
#include "value_holder.h"


namespace nyan {

class ASTMemberValue;
class Member;
class Object;


/**
 * Base class for all possible member values.
 */
class Value {
public:
	Value() = default;
	virtual ~Value() = default;

	/**
	 * Create a value of this type from the AST.
	 *
	 * @param[in]  target_type Type of the value's member.
	 * @param[in]  astmembervalue Value representation as the ASTMemberValue.
	 * @param[out] objs_in_values Pointer to the list of object identifiers in
	 *     values at load time.
	 * @param[in]  get_fqon Function for retrieving an object identifier from
	 *     an IDToken.
	 * @param[in]  get_obj_lin Function for retrieving the object linearization
	 *     for an object.
	 *
	 * @return ValueHolder with shared pointer to the value.
	 */
	static ValueHolder from_ast(const Type &target_type,
	                            const ASTMemberValue &astmembervalue,
	                            std::vector<std::pair<fqon_t, Location>> *objs_in_values,
	                            const std::function<fqon_t(const IDToken &)> &get_fqon,
	                            const std::function<std::vector<fqon_t>(const fqon_t &)> &get_obj_lin);

	/**
	 * Get a copy of this Value.
	 *
	 * @return ValueHolder with shared pointer to the value.
	 */
	virtual ValueHolder copy() const = 0;

	/**
	 * Apply a given change to this value. It will be modified
	 * by the change member's operation and value.
	 *
	 * @param change Member that is applied.
	 */
	void apply(const Member &change);

	/**
	 * Get the string representation of the value.
	 *
	 * @return String representation of the value.
	 */
	virtual std::string str() const = 0;

	/**
	 * Get the short string representation of the value.
	 *
	 * @return Short string representation of the value.
	 */
	virtual std::string repr() const = 0;

	/**
	 * Get the hash of the value.
	 *
	 * @return Hash of the value.
	 */
	virtual size_t hash() const = 0;

	/**
	 * Get the basic type of this value. This means the primitive
	 * and the composite type are provided, but not composite element
	 * types and object targets, because types are not inferred for values.
	 *
	 * @return Basic type of the value.
	 */
	virtual const BasicType &get_type() const = 0;

	/**
	 * Get the set of allowed operations with a given type. This means
	 * the allowed operations can be used to assign/manipulate the
	 * member entry with this value.
	 *
	 * @param with_type Type for which allowed operations are retrieved.
	 *
	 * @return Set of allowed operation with the type.
	 */
	virtual const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const = 0;

	/**
	 * Equality comparison for Values. Performs the typeid comparison,
	 * then calls this->equals(other).
	 *
	 * @param other Value that is compared with.
	 *
	 * @return true if the values are equal, else false.
	 */
	bool operator ==(const Value &other) const;

	/**
	 * Inequality comparison for Values.
	 *
	 * @param other Value that is compared with.
	 *
	 * @return true if the values are not equal, else false.
	 */
	bool operator !=(const Value &other) const;

protected:
	/**
	 * Value-specific comparison function.
	 * It casts the value dynamically, but no type check is performed!
	 *
	 * @param other Value that is compared with.
	 *
	 * @return true if the values are equal, else false.
	 */
	virtual bool equals(const Value &other) const = 0;

	/**
	 * Apply the given change to the value. Internally casts the value dynamically.
	 * A type check for compatibility must be done before calling this function.
	 *
	 * @param value Value that is applied.
	 * @param operation Operation used in the application.
	 *
	 * @return true if the application was successful, else false.
	 */
	virtual bool apply_value(const Value &value, nyan_op operation) = 0;
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
