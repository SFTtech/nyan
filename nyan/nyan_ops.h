// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_OPS_H_
#define NYAN_OPS_H_

#include <unordered_map>

#include "nyan_error.h"

namespace nyan {

class NyanToken;

/**
 * Operation identifiers for all builtin member types
 */
enum class nyan_op {
	ASSIGN,
	ADD,
	SUBTRACT,
	MULTIPLY,
	DIVIDE,
	ADD_ASSIGN,
	SUBTRACT_ASSIGN,
	MULTIPLY_ASSIGN,
	DIVIDE_ASSIGN,
	UNION_ASSIGN,
	INTERSECT_ASSIGN,
	INVALID,
};

/**
 * Return the operator
 */
nyan_op op_from_string(const std::string &str);


/**
 * Return the string representation of
 */
const char *op_to_string(nyan_op op);


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
