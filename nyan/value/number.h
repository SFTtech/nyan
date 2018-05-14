// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include "value.h"


namespace nyan {

class IDToken;

/**
 * Nyan value to store a number.
 */
template <typename T>
class Number : public Value {
public:
	Number(const IDToken &token);
	Number(T value);

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

	operator T() const {
		return this->value;
	}

protected:
	void apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override;

	/**
	 * Actual numerical value.
	 */
	T value;
};


/**
 * Integer storage.
 */
using Int = Number<int64_t>;


/**
 * Floating point data type.
 */
using Float = Number<double>;

} // namespace std
