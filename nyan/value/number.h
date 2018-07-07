// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <functional>

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
	Number(T value)
		:
		value{value} {}

	ValueHolder copy() const override {
		return {std::make_shared<Number>(*this)};
	}

	std::string str() const override {
		return std::to_string(this->value);
	}

	std::string repr() const override {
		return this->str();
	}

	size_t hash() const override {
		return std::hash<T>{}(this->value);
	}

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

	operator T() const {
		return this->value;
	}

	using storage_type = T;
protected:
	void apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override {
		auto &other_val = dynamic_cast<const Number &>(other);
		return this->value == other_val.value;
	}

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

} // namespace nyan
