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
		{"@=", nyan_op::PATCH},
	};

	auto it = str_to_op.find(str);

	if (it == std::end(str_to_op)) {
		return nyan_op::INVALID;
	} else {
		return it->second;
	}
}


nyan_op op_from_token(const NyanToken &token) {
	if (token.type == token_type::OPERATOR) {
		return op_from_string(token.get());
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
