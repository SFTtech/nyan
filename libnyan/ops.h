// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_OPS_H_
#define NYAN_OPS_H_

#include <unordered_map>
#include <unordered_set>

#include "error.h"

namespace nyan {

class NyanToken;

/**
 * Operation identifiers for all builtin member types
 */
enum class nyan_op {
	ADD,
	ADD_ASSIGN,
	ASSIGN,
	DIVIDE,
	DIVIDE_ASSIGN,
	INTERSECT_ASSIGN,
	INVALID,
	MULTIPLY,
	MULTIPLY_ASSIGN,
	PATCH,
	SUBTRACT,
	SUBTRACT_ASSIGN,
	UNION_ASSIGN,
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
 * Return the operator
 */
nyan_op op_from_string(const std::string &str);


/**
 * Return the string representation of a nyan operator.
 */
constexpr const char *op_to_string(nyan_op op) {
	switch (op) {
	case nyan_op::ADD:                return "+";
	case nyan_op::ADD_ASSIGN:         return "+=";
	case nyan_op::ASSIGN:             return "=";
	case nyan_op::DIVIDE:             return "/";
	case nyan_op::DIVIDE_ASSIGN:      return "/=";
	case nyan_op::INTERSECT_ASSIGN:   return "&=";
	case nyan_op::INVALID:            return "=INVALID=";
	case nyan_op::MULTIPLY:           return "*";
	case nyan_op::MULTIPLY_ASSIGN:    return "*=";
	case nyan_op::PATCH:              return "@=";
	case nyan_op::SUBTRACT:           return "-";
	case nyan_op::SUBTRACT_ASSIGN:    return "-=";
	case nyan_op::UNION_ASSIGN:       return "|=";
	}
	return "unhandled nyan_op";
}


/**
 * Create the nyan_op from a token.
 */
nyan_op op_from_token(const NyanToken &token);


/**
 * A nyan operator, to be created from either a token
 */
class NyanOperator {
public:
	NyanOperator(const NyanToken &token);
	virtual ~NyanOperator() = default;

	const nyan_op &get();

protected:
	const nyan_op op;
};


} // namespace nyan

namespace std {

/**
 * Hash for the nyan_op enum class. Thanks C++!
 */
template<>
struct hash<nyan::nyan_op> {
	size_t operator ()(const nyan::nyan_op &x) const {
		return static_cast<size_t>(x);
	}
};

} // namespace std

#endif
