// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_ops.h"

#include "nyan_ast.h"
#include "nyan_token.h"


namespace nyan {

nyan_op op_from_string(const std::string &str) {
	static const std::unordered_map<std::string, nyan_op> str_to_op{
		{"=", nyan_op::ASSIGN},
		{"+", nyan_op::ADD},
		{"-", nyan_op::SUBTRACT},
		{"*", nyan_op::MULTIPLY},
		{"/", nyan_op::DIVIDE},
		{"+=", nyan_op::ADD_ASSIGN},
		{"-=", nyan_op::SUBTRACT_ASSIGN},
		{"*=", nyan_op::MULTIPLY_ASSIGN},
		{"/=", nyan_op::DIVIDE_ASSIGN},
		{"|=", nyan_op::UNION_ASSIGN},
		{"&=", nyan_op::INTERSECT_ASSIGN},
	};

	auto it = str_to_op.find(str);

	if (it == std::end(str_to_op)) {
		return nyan_op::INVALID;
	} else {
		return it->second;
	}
}

const char *op_to_string(nyan_op op) {
	// fak you c++. somebody please find a sane way to purge the redundancy.
	switch (op) {
	case nyan_op::ASSIGN:             return "=";
	case nyan_op::ADD:                return "+";
	case nyan_op::SUBTRACT:           return "-";
	case nyan_op::MULTIPLY:           return "*";
	case nyan_op::DIVIDE:             return "/";
	case nyan_op::ADD_ASSIGN:         return "+=";
	case nyan_op::SUBTRACT_ASSIGN:    return "-=";
	case nyan_op::MULTIPLY_ASSIGN:    return "*=";
	case nyan_op::DIVIDE_ASSIGN:      return "/=";
	case nyan_op::UNION_ASSIGN:       return "|=";
	case nyan_op::INTERSECT_ASSIGN:   return "&=";
	case nyan_op::INVALID:            return "=INVALID=";
	}
}


nyan_op op_from_token(const NyanToken &token) {
	if (token.type == token_type::OPERATOR) {
		return op_from_string(token.value);
	}
	else {
		throw ASTError("expected operator, but got", token);
	}
}

NyanOperator::NyanOperator(const NyanToken &token)
	:
	op{op_from_token(token)} {}

const nyan_op &NyanOperator::get() {
	return this->op;
}


} // namespace nyan
