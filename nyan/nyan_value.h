// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_H_
#define NYAN_NYAN_VALUE_H_

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "nyan_ops.h"
#include "nyan_type.h"

namespace nyan {

class NyanObject;
class NyanMember;


/**
 * Base class for all possible member values.
 */
class NyanValue {
public:
	NyanValue();
	virtual ~NyanValue() = default;

	/**
	 * Return a copy of this NyanValue.
	 */
	virtual std::unique_ptr<NyanValue> copy() const = 0;

	/**
	 * Apply the given change to this value.
	 * The operation and value are applied.
	 */
	void apply(const NyanMember *change);

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
	virtual const nyan_basic_type &get_type() const = 0;

	/**
	 * Comparison for NyanValues.
	 * Performs the typeid comparison, then calls this->equals(other).
	 */
	bool operator ==(const NyanValue &other) const;

	/**
	 * Inequality operator, it's just "not ==".
	 */
	bool operator !=(const NyanValue &other) const;

protected:
	/**
	 * Value-specific comparison function.
	 * It casts the value dynamically, but no type check is performed!
	 */
	virtual bool equals(const NyanValue &other) const = 0;

	/**
	 * Apply the given change to the value.
	 * Internally casts the value dynamically,
	 * the type check must be done before calling this function.
	 */
	virtual void apply_value(const NyanValue *value, nyan_op operation) = 0;

	/**
	 * Allowed operations for this value type with a given operand.
	 */
	virtual const std::unordered_set<nyan_op> &allowed_operations(nyan_basic_type value_type) const = 0;
};

} // namespace nyan


namespace std {

/**
 * Hash for NyanValues.
 */
template<>
struct hash<nyan::NyanValue *> {
	size_t operator ()(const nyan::NyanValue *val) const {
		return val->hash();
	}
};

} // namespace std

#endif
