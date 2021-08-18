// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <functional>
#include <optional>

#include "value.h"

#include "../config.h"


namespace nyan {

class IDToken;

class NumberBase : public Value {
	template <typename T>
	friend class Number;

public:
	virtual ~NumberBase() = default;

	/** Checks if the value is positive or negative infinity. */
	virtual bool is_infinite() const = 0;

	/** Checks if the value is positive infinity */
	virtual bool is_infinite_positive() const = 0;

	/** is this number zero? */
	virtual bool is_zero() const = 0;

	/** Check if the number is positive. */
	virtual bool is_positive() const = 0;

protected:
	/**
	 * get this number as float
	 * does _not_ do infinity conversions.
	 */
	virtual value_float_t as_float() const = 0;

	/**
	 * get this number as int
	 * does _not_ do infinity conversions.
	 */
	virtual value_int_t as_int() const = 0;
};


/**
 * Nyan value to store a number.
 */
template <typename T>
class Number : public NumberBase {
public:
	/** the actual number is stored as a configurable type */
	using storage_type = T;

	/**
	 * result of an infinity calculation.
	 * this and other mean the current operand values.
	 */
	enum class infinity_action {
		THIS,     //!< keep the value of this number
		OTHER,    //!< take the value of the other number
		INF_POS,  //!< set this to positive infinite
		INF_NEG,  //!< set this to negative infinite
		ZERO,     //!< set this to zero
	};

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

	T get() const {
		return *this;
	}

	/**
	 * Checks if the value is positive or negative infinity.
	 */
	bool is_infinite() const override;

	/**
	 * Checks if the value is positive infinity.
	 */
	bool is_infinite_positive() const override;

	/**
	 * Check if the number is zero.
	 */
	bool is_zero() const override;

	/**
	 * Check if the number is positive.
	 */
	bool is_positive() const override;

	/**
	 * Get positive infinity value for this number.
	 */
	constexpr static T infinite_pos();

	/**
	 * Get negative infinity value for this number.
	 */
	constexpr static T infinite_neg();

	/**
	 * Calculates the value that is assigned when one of the operands
	 * of apply_value() is infinity. This is Maybe {pos_inf, neg_inf, this, other}.
	 * In case the result is NaN, return Nothing.
	 */
	std::optional<infinity_action> handle_infinity(const NumberBase &other, nyan_op operation);

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

	operator T() const {
		return this->value;
	}

protected:
	bool apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override {
		auto &other_val = dynamic_cast<const Number &>(other);
		return this->value == other_val.value;
	}

	/** get this number as float */
	value_float_t as_float() const override;

	/** get this number as int */
	value_int_t as_int() const override;


	/**
	 * Actual numerical value.
	 */
	T value;
};


/**
 * Integer storage.
 */
using Int = Number<value_int_t>;


/**
 * Floating point data type.
 */
using Float = Number<value_float_t>;



template<>
constexpr Float::storage_type
Number<Float::storage_type>::infinite_pos() {
	return std::numeric_limits<Float::storage_type>::infinity();
}

template<>
constexpr Float::storage_type
Number<Float::storage_type>::infinite_neg() {
	return -std::numeric_limits<Float::storage_type>::infinity();
}


template<>
constexpr Int::storage_type
Number<Int::storage_type>::infinite_pos() {
	return std::numeric_limits<Int::storage_type>::max();
}

template<>
constexpr Int::storage_type
Number<Int::storage_type>::infinite_neg() {
	return std::numeric_limits<Int::storage_type>::min();
}


} // namespace nyan
