// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_H_
#define NYAN_NYAN_VALUE_H_

#include <list>
#include <memory>
#include <string>
#include <unordered_set>

#include "nyan_ops.h"

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
	 * @returns if the parent values were already included because of caching
	 */
	bool apply(const NyanMember *change);

	/**
	 * Sting representation of the value.
	 */
	virtual std::string str() const = 0;

	/**
	 * Hash function for the value.
	 */
	virtual size_t hash() const = 0;

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
	 * Allowed operations for this value type.
	 */
	std::unordered_set<nyan_op> operations;
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

namespace nyan {


/**
 * Nyan value to store text.
 */
class NyanText : public NyanValue {
public:
	NyanText(const std::string &value);

	std::unique_ptr<NyanValue> copy() const override;
	std::string str() const override;
	size_t hash() const override;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	bool equals(const NyanValue &other) const override;
	std::string value;
};


/**
 * Nyan value to store a number.
 */
class NyanInt : public NyanValue {
public:
	NyanInt(int64_t value);

	std::unique_ptr<NyanValue> copy() const override;
	std::string str() const override;
	size_t hash() const override;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	bool equals(const NyanValue &other) const override;
	int64_t value;
};


} // namespace nyan

#endif
