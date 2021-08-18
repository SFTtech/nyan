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
				this->value = this->infinite_pos();
			}
			else {
				this->value = this->infinite_neg();
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
				this->value = this->infinite_pos();
			}
			else {
				this->value = this->infinite_neg();
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


template <typename T>
bool Number<T>::is_infinite() const {
	return (this->value == this->infinite_pos() or
	        this->value == this->infinite_neg());
}

template <typename T>
bool Number<T>::is_infinite_positive() const {
	return this->value == this->infinite_pos();
}

template <typename T>
bool Number<T>::is_zero() const {
	return this->value == 0;
}

template <typename T>
bool Number<T>::is_positive() const {
	return this->value > 0;
}


template <typename T>
bool Number<T>::apply_value(const Value &value, nyan_op operation) {
	// apply the given number to `this`
	auto applier = [this](auto operand, nyan_op operation) {
		switch (operation) {
		case nyan_op::ASSIGN:
			this->value = operand; break;

		case nyan_op::ADD_ASSIGN:
			this->value += operand; break;

		case nyan_op::SUBTRACT_ASSIGN:
			this->value -= operand; break;

		case nyan_op::MULTIPLY_ASSIGN:
			this->value *= operand; break;

		case nyan_op::DIVIDE_ASSIGN:
			this->value /= operand; break;

		default:
			throw InternalError{"unknown operation requested"};
		}
	};

	// apply the given number to `this`, and convert it before
	auto apply_number_convert = [&applier](const NumberBase &number, nyan_op operation) {
		if constexpr (std::is_same_v<Number<T>, Int>) {
			if (typeid(const Float &) == typeid(number)) {
				applier(number.as_float(), operation);
			}
			else if (typeid(const Int &) == typeid(number)) {
				applier(number.as_int(), operation);
			}
			else {
				throw InternalError{"unknown number type to be applied"};
			}
		}
		else if constexpr (std::is_same_v<Number<T>, Float>) {
			applier(number.as_float(), operation);
		}
		else {
			util::match_failure();
		}
	};

	// `this` is either float or int, and `value` can also be either.

	const NumberBase *number = dynamic_cast<const NumberBase *>(&value);
	if (unlikely(number == nullptr)) {
		throw InternalError("expected Number instance for operation, but got"
		                    + util::typestring(&value));
	}

	// regular numbers without infinity
	if (not (this->is_infinite() or number->is_infinite())) {
		apply_number_convert(*number, operation);
	}
	else {
		// handle infinity values
		auto inf_result = this->handle_infinity(*number, operation);

		if (inf_result.has_value()) {
			switch (inf_result.value()) {
			case infinity_action::OTHER:
				apply_number_convert(*number, nyan_op::ASSIGN);
				break;

			case infinity_action::THIS:
				// just keep our value
				break;

			case infinity_action::INF_POS:
				this->value = this->infinite_pos();
				break;

			case infinity_action::INF_NEG:
				this->value = this->infinite_neg();
				break;

			case infinity_action::ZERO:
				this->value = 0;
				break;
			}
		}
		else {
			// the operation failed, so the whole patch transaction has to abort
			// or we figure out some other way to handle infinity failures
			// TODO: remove the exception once transactions can fail properly
			throw Error{"invalid infinity application"};
			return false;
		}
	}

	return true;
}


// TODO: instead of an optional, we should return error messages..
// but for that we should already have some kind of in-transaction-error-callback
// mechanism
template <typename T>
std::optional<typename Number<T>::infinity_action>
Number<T>::handle_infinity(const NumberBase &other, nyan_op operation) {

	auto inf_by_other = [&other](bool invert=false) {
		if (other.is_infinite_positive() ^ invert) {
			return infinity_action::INF_POS;
		}
		return infinity_action::INF_NEG;
	};

	// Both values are infinite
	if (this->is_infinite() and other.is_infinite()) {
		switch (operation) {
		case nyan_op::ASSIGN:
			return inf_by_other();

		case nyan_op::ADD_ASSIGN: {
			if (this->is_infinite_positive() == other.is_infinite_positive()) {
				return inf_by_other();
			}
			// adding two inf values with different sign not permitted
			return {};
		}

		case nyan_op::SUBTRACT_ASSIGN: {
			if (this->is_infinite_positive() == other.is_infinite_positive()) {
				return inf_by_other();
			}
			// subtracting two inf values with different sign not permitted
			return {};
		}

		case nyan_op::MULTIPLY_ASSIGN: {
			if (this->is_infinite_positive() ^ other.is_infinite_positive()) {
				return infinity_action::INF_NEG;
			}
			return infinity_action::INF_POS;
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
			return infinity_action::OTHER;

		case nyan_op::ADD_ASSIGN:
		case nyan_op::SUBTRACT_ASSIGN:
			return infinity_action::THIS;

		case nyan_op::MULTIPLY_ASSIGN: {
			if (other.is_zero()) {
				// multiplying inf with 0 not permitted
				return {};
			}

			// other_inf xor !this_inf
			return inf_by_other(not this->is_positive());
		}

		case nyan_op::DIVIDE_ASSIGN: {
			if (other.is_zero()) {
				// dividing inf by 0 not permitted
				return {};
			}
			return infinity_action::THIS;
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
			return inf_by_other();

		case nyan_op::SUBTRACT_ASSIGN:
			return inf_by_other(true);

		case nyan_op::MULTIPLY_ASSIGN: {
			if (this->is_zero()) {
				// multiplying inf with 0 not permitted
				return {};
			}

			return inf_by_other(not this->is_positive());
		}

		case nyan_op::DIVIDE_ASSIGN:
			return infinity_action::ZERO;

		default:
			throw InternalError{"unknown operation requested"};
		}
	}
	throw InternalError{"expected at least one infinite operand"};
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


template <typename T>
value_float_t Number<T>::as_float() const {
	// using the brave assumption that the int type max values are always lower
	// than the floatingpoint types max values
	return static_cast<value_float_t>(this->value);
}

template <>
value_int_t Int::as_int() const {
	return static_cast<value_int_t>(this->value);
}

template <>
value_int_t Float::as_int() const {
	// float -> int might overflow...
	if (unlikely(
		    (static_cast<value_float_t>(std::numeric_limits<value_int_t>::max()) < this->value) or
		    (static_cast<value_float_t>(std::numeric_limits<value_int_t>::min()) > this->value)
	    )) {

		// we should handle this better instead of hard-crashing...
		throw Error{"float to int conversion impossible since value wouldn't fit"};
	}

	return static_cast<value_int_t>(this->value);
}


// explicit instantiations
template class Number<value_int_t>; // Int
template class Number<value_float_t>; // Float

} // namespace nyan
