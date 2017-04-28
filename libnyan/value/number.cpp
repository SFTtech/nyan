// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "number.h"

#include <string>
#include <typeinfo>

#include "../error.h"
#include "../ops.h"
#include "../token.h"


namespace nyan {

template<>
NyanInt::Number(const Token &token) {
	try {
		this->value = std::stoll(token.get(), nullptr, 0);
	}
	catch (std::invalid_argument &) {
		throw InternalError{"int token was not an int"};
	}
	catch (std::out_of_range &) {
		throw FileError{token, "number out of range"};
	}
}


template<>
NyanFloat::Number(const Token &token) {
	try {
		this->value = std::stod(token.get());
	}
	catch (std::invalid_argument &) {
		throw InternalError{"float token was not a float"};
	}
	catch (std::out_of_range &) {
		throw FileError{token, "number out of range"};
	}
}


template <typename T>
Number<T>::Number(T value)
	:
	value{value} {}


template <typename T>
std::unique_ptr<Value> Number<T>::copy() const {
	return std::make_unique<Number>(dynamic_cast<const Number &>(*this));
}


template <typename T>
void Number<T>::apply_value(const Value *value, nyan_op operation) {
	const Number *change = dynamic_cast<const Number *>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change->value; break;

	case nyan_op::ADD_ASSIGN:
		this->value += change->value; break;

	case nyan_op::SUBTRACT_ASSIGN:
		this->value -= change->value; break;

	case nyan_op::MULTIPLY_ASSIGN:
		this->value *= change->value; break;

	case nyan_op::DIVIDE_ASSIGN:
		this->value /= change->value; break;

	default:
		throw Error{"unknown operation requested"};
	}
}


template <typename T>
std::string Number<T>::str() const {
	std::ostringstream builder;
	builder << this->value;
	return builder.str();
}


template <typename T>
std::string Number<T>::repr() const {
	return this->str();
}


template <typename T>
size_t Number<T>::hash() const {
	return this->value;
}


template <typename T>
bool Number<T>::equals(const Value &other) const {
	auto &other_val = dynamic_cast<const Number &>(other);
	return this->value == other_val.value;
}


template <typename T>
const std::unordered_set<nyan_op> &
Number<T>::allowed_operations(nyan_basic_type value_type) const {

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::MULTIPLY_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::DIVIDE_ASSIGN,
	};

	const nyan_primitive_type &type = value_type.primitive_type;

	// all allowed number types
	if (type == nyan_primitive_type::FLOAT or
	    type == nyan_primitive_type::INT) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


template<>
const nyan_basic_type &NyanInt::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::INT,
		nyan_container_type::SINGLE
	};
	return type;
}


template<>
const nyan_basic_type &NyanFloat::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::FLOAT,
		nyan_container_type::SINGLE
	};

	return type;
}


} // namespace nyan
