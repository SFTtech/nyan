// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

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
			throw InternalError{"unknown operation requested"};
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
			if (change_value.has_value()) {
				applier(this->value, change_value.value(), nyan_op::ASSIGN);
			}
			else {
				// return false;
			}
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
			if (change_value.has_value()) {
				applier(this->value, change_value.value(), nyan_op::ASSIGN);
			}
			else {
				// return false;
			}
		}
	}
	else {
		throw InternalError("expected Number instance for operation, but got"
		                    + std::string(typeid(value).name()));
	}
}


template <typename T>
std::optional<typename Number<T>::storage_type>
Number<T>::handle_infinity(const Number<T> &other, nyan_op operation) {
	auto inf_pos = [] () -> storage_type {
		if constexpr (std::is_same_v<Number<T>, Int>) {
			return INT_POS_INF;
		}
		else {
			return FLOAT_POS_INF;
		}
	};
	auto inf_neg = [] () -> storage_type {
		if constexpr (std::is_same_v<Number<T>, Int>) {
			return INT_POS_INF;
		}
		else {
			return FLOAT_POS_INF;
		}
	};

	// Both values are infinite
	if (this->is_infinite() and other.is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
			return other.get();

		case nyan_op::ADD_ASSIGN: {
			if (this->value == other.get()) {
				return this->value;
			}
			// adding two inf values with different sign not permitted
			return {};
		}

		case nyan_op::SUBTRACT_ASSIGN: {
			if (this->value != other.get()) {
				return this->value;
			}
			// subtracting two inf values with different sign not permitted
			return {};
		}

		case nyan_op::MULTIPLY_ASSIGN:{
			if (this->value == other.get()) {
				return inf_pos();
			}
			return inf_neg();
		}

		case nyan_op::DIVIDE_ASSIGN:
			// dividing two inf values not permitted
			return {};

		default:
			throw InternalError{"unknown operation requested"};
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
				// multiplying inf with 0 not permitted
				return {};
			}
			else if (other.get() > 0) {
				return this->value;
			}
			else if (this->value > 0) {
				return inf_neg();
			}
			return inf_pos();
		}

		case nyan_op::DIVIDE_ASSIGN:{
			if (other.get() == 0) {
				// dividing inf by 0 not permitted
				return {};
			}
			return this->value;
		}

		default:
			throw InternalError{"unknown operation requested"};
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
				return inf_neg();
			}
			return inf_pos();
		}

		case nyan_op::MULTIPLY_ASSIGN:{
			if (this->value == 0) {
				// multiplying inf with 0 not permitted
				return {};
			}
			else if (this->value > 0) {
				return other.get();
			}
			else if (other.get() > 0) {
				return inf_neg();
			}
			return inf_pos();
		}

		case nyan_op::DIVIDE_ASSIGN:
			return 0;

		default:
			throw InternalError{"unknown operation requested"};
		}
	}
	throw InternalError{"expected at least one infinite operand"};
}


template <typename T>
const std::unordered_set<nyan_op> &
Number<T>::allowed_operations(const Type &with_type) const {

	const static std::unordered_set<nyan_op> none_ops{
		nyan_op::ASSIGN,
	};

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

	case primitive_t::NONE:
		return none_ops;

	default:
		return no_nyan_ops;
	}
}


template<>
const BasicType &Int::get_type() const {
	constexpr static BasicType type{
		primitive_t::INT,
		composite_t::SINGLE
	};
	return type;
}


template<>
const BasicType &Float::get_type() const {
	constexpr static BasicType type{
		primitive_t::FLOAT,
		composite_t::SINGLE
	};

	return type;
}

// explicit instantiations
template class Number<value_int_t>; // Int
template class Number<value_float_t>; // Float

} // namespace nyan
