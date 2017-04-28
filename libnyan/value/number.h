// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_NUMBER_H_
#define NYAN_NYAN_VALUE_NUMBER_H_

#include "value.h"


namespace nyan {

/**
 * Nyan value to store a number.
 */
template <typename T>
class Number : public Value {
public:
	Number(const Token &token);
	Number(T value);

	std::unique_ptr<Value> copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	const std::unordered_set<nyan_op> &allowed_operations(nyan_basic_type value_type) const override;
	const nyan_basic_type &get_type() const override;

protected:
	void apply_value(const Value *value, nyan_op operation) override;
	bool equals(const Value &other) const override;

	/**
	 * Actual numerical value.
	 */
	T value;
};


/**
 * Integer storage.
 */
using NyanInt = Number<int64_t>;


/**
 * Floating point data type.
 */
using NyanFloat = Number<double>;

} // namespace std

#endif
