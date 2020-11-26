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

static void check_token(const IDToken &token, std::vector<token_type> expected) {

	using namespace std::string_literals;

	if (unlikely(not (std::find(expected.begin(),
						   		expected.end(),
						    	token.get_type()) != expected.end()))) {
		throw LangError{
			token,
			"invalid value for number, expected "s
			+ util::strjoin(
				" or ",
				expected,
				[] (const auto &token_type) {
					return token_type_str(token_type);
				}
			)
		};
	}
}


template<>
Int::Number(const IDToken &token) {

	const static std::vector<token_type> expected{
		token_type::INT,
		token_type::INF,
	};

	check_token(token, expected);

	try {
		if (token.get_type() == token_type::INF) {
			if (token.str() == "inf") {
				this->value = INT_POS_INF;
			}
			else {
				this->value = INT_NEG_INF;
			}
		}
		else {
			this->value = std::stoll(token.get_first(), nullptr, 0);
		}
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

	const static std::vector<token_type> expected{
		token_type::FLOAT,
		token_type::INF,
	};

	check_token(token, expected);

	try {
		if (token.get_type() == token_type::INF) {
			if (token.str() == "inf") {
				this->value = FLOAT_POS_INF;
			}
			else {
				this->value = FLOAT_NEG_INF;
			}
		}
		else {
			this->value = std::stod(token.get_first());
		}
	}
	catch (std::invalid_argument &) {
		throw InternalError{"float token was not a float"};
	}
	catch (std::out_of_range &) {
		throw LangError{token, "number out of range"};
	}
}


template<>
bool Number<typename Int::storage_type>::is_infinite() const {
	return (this->value == INT_POS_INF or this->value == INT_NEG_INF);
}


template<>
bool Number<typename Float::storage_type>::is_infinite() const {
	return (this->value == FLOAT_POS_INF or this->value == FLOAT_NEG_INF);
}


template <typename T>
void Number<T>::apply_value(const Value &value, nyan_op operation) {
	auto applier = [](auto &member_value, auto operand, nyan_op operation) {
		switch (operation) {
		case nyan_op::ASSIGN:
			member_value = operand; break;

		case nyan_op::ADD_ASSIGN:
			member_value += operand; break;

		case nyan_op::SUBTRACT_ASSIGN:
			member_value -= operand; break;

		case nyan_op::MULTIPLY_ASSIGN:
			member_value *= operand; break;

		case nyan_op::DIVIDE_ASSIGN:
			member_value /= operand; break;

		default:
			throw Error{"unknown operation requested"};
		}
	};

	if (typeid(Float&) == typeid(value)) {
		const Float &change = dynamic_cast<const Float &>(value);

		if (not (this->is_infinite() or change.is_infinite())) {
			applier(this->value, change.get(), operation);
		}
		else {
			Float left = static_cast<Float>(*this);
			auto change_value = left.handle_infinity(change, operation);
			applier(this->value, change_value, nyan_op::ASSIGN);
		}
	}
	else if (typeid(Int&) == typeid(value)) {
		const Int &change = dynamic_cast<const Int &>(value);

		if (not (this->is_infinite() or change.is_infinite())) {
			applier(this->value, change.get(), operation);
		}
		else {
			Int left = static_cast<Int>(*this);
			auto change_value = left.handle_infinity(change, operation);
			applier(this->value, change_value, nyan_op::ASSIGN);
		}
	}
	else {
		throw InternalError("expected Number instance for operation, but got"
					  		+ std::string(typeid(value).name()));
	}
}


template<>
Int::storage_type Int::handle_infinity(const Int &other, nyan_op operation) {

	// Both values are infinite
	if (this->is_infinite() and other.is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
			return other.get();

		case nyan_op::ADD_ASSIGN: {
			if (this->value == other.get()) {
				return this->value;
			}
			throw Error{"adding two inf values with different sign not permitted"};
		}

		case nyan_op::SUBTRACT_ASSIGN: {
			if (this->value != other.get()) {
				return this->value;
			}
			throw Error{"subtracting two inf values with different sign not permitted"};
		}

		case nyan_op::MULTIPLY_ASSIGN:{
			if (this->value == other.get()) {
				return INT_POS_INF;
			}
			return INT_NEG_INF;
		}

		case nyan_op::DIVIDE_ASSIGN:
			throw Error{"dividing two inf values not permitted"};

		default:
			throw Error{"unknown operation requested"};
		}
	}
	// Only left operand is infinite
	else if (this->is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
			return other.get();

		case nyan_op::ADD_ASSIGN:
		case nyan_op::SUBTRACT_ASSIGN:
			return this->value;

		case nyan_op::MULTIPLY_ASSIGN: {
			if (other.get() == 0) {
				throw Error{"multiplying inf with 0 not permitted"};
			}
			else if (other.get() > 0) {
				return this->value;
			}
			else if (this->value > 0) {
				return INT_NEG_INF;
			}
			return INT_POS_INF;
		}

		case nyan_op::DIVIDE_ASSIGN:{
			if (other.get() == 0) {
				throw Error{"dividing inf by 0 not permitted"};
			}
			return this->value;
		}

		default:
			throw Error{"unknown operation requested"};
		}
	}
	// Only right operand is infinite
	else if (other.is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
		case nyan_op::ADD_ASSIGN:
			return other.get();

		case nyan_op::SUBTRACT_ASSIGN: {
			if (other.get() > 0) {
				return INT_NEG_INF;
			}
			return INT_POS_INF;
		}

		case nyan_op::MULTIPLY_ASSIGN:{
			if (this->value == 0) {
				throw Error{"multiplying inf with 0 not permitted"};
			}
			else if (this->value > 0) {
				return other.get();
			}
			else if (other.get() > 0) {
				return INT_NEG_INF;
			}
			return INT_POS_INF;
		}

		case nyan_op::DIVIDE_ASSIGN:
			return 0;

		default:
			throw Error{"unknown operation requested"};
		}
	}
	throw InternalError("expected at least one infinite operand");
}


template<>
Float::storage_type Float::handle_infinity(const Float &other, nyan_op operation) {

	// Both values are infinite
	if (this->is_infinite() and other.is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
			return other.get();

		case nyan_op::ADD_ASSIGN: {
			if (this->value == other.get()) {
				return this->value;
			}
			throw Error{"adding two inf values with different sign not permitted"};
		}

		case nyan_op::SUBTRACT_ASSIGN: {
			if (this->value != other.get()) {
				return this->value;
			}
			throw Error{"subtracting two inf values with different sign not permitted"};
		}

		case nyan_op::MULTIPLY_ASSIGN:{
			if (this->value == other.get()) {
				return FLOAT_POS_INF;
			}
			return INT_NEG_INF;
		}

		case nyan_op::DIVIDE_ASSIGN:
			throw Error{"dividing two inf values not permitted"};

		default:
			throw Error{"unknown operation requested"};
		}
	}
	// Only left operand is infinite
	else if (this->is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
			return other.get();

		case nyan_op::ADD_ASSIGN:
		case nyan_op::SUBTRACT_ASSIGN:
			return this->value;

		case nyan_op::MULTIPLY_ASSIGN: {
			if (other.get() == 0) {
				throw Error{"multiplying inf with 0 not permitted"};
			}
			else if (other.get() > 0) {
				return this->value;
			}
			else if (this->value > 0) {
				return FLOAT_NEG_INF;
			}
			return FLOAT_POS_INF;
		}

		case nyan_op::DIVIDE_ASSIGN:{
			if (other.get() == 0) {
				throw Error{"dividing inf by 0 not permitted"};
			}
			return this->value;
		}

		default:
			throw Error{"unknown operation requested"};
		}
	}
	// Only right operand is infinite
	else if (other.is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
		case nyan_op::ADD_ASSIGN:
			return other.get();

		case nyan_op::SUBTRACT_ASSIGN: {
			if (other.get() > 0) {
				return FLOAT_NEG_INF;
			}
			return FLOAT_POS_INF;
		}

		case nyan_op::MULTIPLY_ASSIGN:{
			if (this->value == 0) {
				throw Error{"multiplying inf with 0 not permitted"};
			}
			else if (this->value > 0) {
				return other.get();
			}
			else if (other.get() > 0) {
				return FLOAT_NEG_INF;
			}
			return FLOAT_POS_INF;
		}

		case nyan_op::DIVIDE_ASSIGN:
			return 0;

		default:
			throw Error{"unknown operation requested"};
		}
	}
	throw InternalError("expected at least one infinite operand");
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
		composite_t::NONE
	};
	return type;
}


template<>
const BasicType &Float::get_type() const {
	constexpr static BasicType type{
		primitive_t::FLOAT,
		composite_t::NONE
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
