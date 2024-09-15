// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <unordered_map>
#include <unordered_set>

#include "error.h"

namespace nyan {

class Token;

/**
 * Operation identifiers for all builtin member types
 */
enum class nyan_op {
	INVALID,
	ADD,
	ADD_ASSIGN,
	ASSIGN,
	DIVIDE,
	DIVIDE_ASSIGN,
	INTERSECT_ASSIGN,
	MULTIPLY,
	MULTIPLY_ASSIGN,
	SUBTRACT,
	SUBTRACT_ASSIGN,
	UNION_ASSIGN,
};


/**
 * Inheritance modification operations.
 */
enum class inher_change_t {
	ADD_FRONT,
	ADD_BACK
};


/**
 * Convenience variable to be used whenever no operation is allowed.
 * It comes in handy as the allowance sets are const static function
 * variables normally, and the function returns a reference.
 * As this variable exists globally, we can return a reference
 * without redefining an empty set again and again.
 */
extern const std::unordered_set<nyan_op> no_nyan_ops;


/**
 * Get a string representation of a nyan operation.
 *
 * @param op Nyan operation.
 *
 * @return Char array with the string representation of the nyan operation.
 */
constexpr const char *op_to_string(nyan_op op) {
	switch (op) {
	case nyan_op::ADD:
		return "+";
	case nyan_op::ADD_ASSIGN:
		return "+=";
	case nyan_op::ASSIGN:
		return "=";
	case nyan_op::DIVIDE:
		return "/";
	case nyan_op::DIVIDE_ASSIGN:
		return "/=";
	case nyan_op::INTERSECT_ASSIGN:
		return "&=";
	case nyan_op::INVALID:
		return "=INVALID=";
	case nyan_op::MULTIPLY:
		return "*";
	case nyan_op::MULTIPLY_ASSIGN:
		return "*=";
	case nyan_op::SUBTRACT:
		return "-";
	case nyan_op::SUBTRACT_ASSIGN:
		return "-=";
	case nyan_op::UNION_ASSIGN:
		return "|=";
	}
	return "unhandled nyan_op";
}


/**
 * Get a nyan operation from a string of an operator.
 *
 * @param str String with an operator.
 *
 * @return A nyan operation.
 */
nyan_op op_from_string(const std::string &str);


/**
 * Get a nyan operation from a token of an operator.
 *
 * @param str Token with an operator string.
 *
 * @return A nyan operation.
 */
nyan_op op_from_token(const Token &token);


/**
 * A nyan operator, to be created from either a token
 *
 * TODO: What is this?
 */
class Operator {
public:
	Operator(const Token &token);
	virtual ~Operator() = default;

	const nyan_op &get();

protected:
	const nyan_op op;
};


} // namespace nyan

namespace std {

/**
 * Hash for the nyan_op enum class. Thanks C++!
 */
template <>
struct hash<nyan::nyan_op> {
	size_t operator()(const nyan::nyan_op &x) const {
		return static_cast<size_t>(x);
	}
};

} // namespace std
