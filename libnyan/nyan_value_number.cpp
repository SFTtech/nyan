// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_number.h"

#include <string>
#include <typeinfo>

#include "nyan_error.h"
#include "nyan_ops.h"
#include "nyan_token.h"


namespace nyan {

template<>
NyanInt::NyanNumber(const NyanToken &token) {
	try {
		this->value = std::stoll(token.get(), nullptr, 0);
	}
	catch (std::invalid_argument &) {
		throw NyanInternalError{"int token was not an int"};
	}
	catch (std::out_of_range &) {
		throw NyanFileError{token, "number out of range"};
	}
}


template<>
NyanFloat::NyanNumber(const NyanToken &token) {
	try {
		this->value = std::stod(token.get());
	}
	catch (std::invalid_argument &) {
		throw NyanInternalError{"float token was not a float"};
	}
	catch (std::out_of_range &) {
		throw NyanFileError{token, "number out of range"};
	}
}


template <typename T>
NyanNumber<T>::NyanNumber(T value)
	:
	value{value} {}


template <typename T>
std::unique_ptr<NyanValue> NyanNumber<T>::copy() const {
	return std::make_unique<NyanNumber>(dynamic_cast<const NyanNumber &>(*this));
}


template <typename T>
void NyanNumber<T>::apply_value(const NyanValue *value, nyan_op operation) {
	const NyanNumber *change = dynamic_cast<const NyanNumber *>(value);

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
		throw NyanError{"unknown operation requested"};
	}
}


template <typename T>
std::string NyanNumber<T>::str() const {
	std::ostringstream builder;
	builder << this->value;
	return builder.str();
}


template <typename T>
std::string NyanNumber<T>::repr() const {
	return this->str();
}


template <typename T>
size_t NyanNumber<T>::hash() const {
	return this->value;
}


template <typename T>
bool NyanNumber<T>::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanNumber &>(other);
	return this->value == other_val.value;
}


template <typename T>
const std::unordered_set<nyan_op> &
NyanNumber<T>::allowed_operations(nyan_basic_type value_type) const {

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
