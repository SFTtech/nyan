// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "number.h"

#include <string>
#include <typeinfo>

#include "../compiler.h"
#include "../lang_error.h"
#include "../id_token.h"
#include "../ops.h"
#include "../token.h"
#include "../util.h"


namespace nyan {

static void check_token(const IDToken &token, token_type expected) {

	using namespace std::string_literals;

	if (unlikely(token.get_type() != expected)) {
		throw LangError{
			token,
			"invalid value for number, expected "s
			+ token_type_str(expected)
		};
	}
}


template<>
Int::Number(const IDToken &token) {

	check_token(token, token_type::INT);

	try {
		this->value = std::stoll(token.get_first(), nullptr, 0);
	}
	catch (std::invalid_argument &) {
		throw InternalError{"int token was not an int"};
	}
	catch (std::out_of_range &) {
		throw LangError{token, "number out of range"};
	}
}


template<>
Float::Number(const IDToken &token) {

	check_token(token, token_type::FLOAT);

	try {
		this->value = std::stod(token.get_first());
	}
	catch (std::invalid_argument &) {
		throw InternalError{"float token was not a float"};
	}
	catch (std::out_of_range &) {
		throw LangError{token, "number out of range"};
	}
}


template <typename T>
void Number<T>::apply_value(const Value &value, nyan_op operation) {
	const Number &change = dynamic_cast<const Number &>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change.value; break;

	case nyan_op::ADD_ASSIGN:
		this->value += change.value; break;

	case nyan_op::SUBTRACT_ASSIGN:
		this->value -= change.value; break;

	case nyan_op::MULTIPLY_ASSIGN:
		this->value *= change.value; break;

	case nyan_op::DIVIDE_ASSIGN:
		this->value /= change.value; break;

	default:
		throw Error{"unknown operation requested"};
	}
}


template <typename T>
const std::unordered_set<nyan_op> &
Number<T>::allowed_operations(const Type &with_type) const {

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::MULTIPLY_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::DIVIDE_ASSIGN,
	};

	// all allowed number types
	switch (with_type.get_primitive_type()) {
	case primitive_t::FLOAT:
	case primitive_t::INT:
		return ops;
	default:
		return no_nyan_ops;
	}
}


template<>
const BasicType &Int::get_type() const {
	constexpr static BasicType type{
		primitive_t::INT,
		container_t::SINGLE
	};
	return type;
}


template<>
const BasicType &Float::get_type() const {
	constexpr static BasicType type{
		primitive_t::FLOAT,
		container_t::SINGLE
	};

	return type;
}

// explicit instantiation of member functions
template
void Number<typename Int::storage_type>::apply_value(
	const Value &value, nyan_op operation);

template
void Number<typename Float::storage_type>::apply_value(
	const Value &value, nyan_op operation);

template
const std::unordered_set<nyan_op> &
Number<typename Int::storage_type>::allowed_operations(
	const Type &with_type) const;

template
const std::unordered_set<nyan_op> &
Number<typename Float::storage_type>::allowed_operations(
	const Type &with_type) const;

} // namespace nyan
