// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "boolean.h"

#include <typeinfo>

#include "../compiler.h"
#include "../lang_error.h"
#include "../util.h"
#include "../token.h"


namespace nyan {

Boolean::Boolean(const bool &value)
	:
	value{value} {}


Boolean::Boolean(const IDToken &token) {

	if (unlikely(token.get_type() != token_type::ID)) {
		throw LangError{
			token,
			"invalid value for boolean"
		};
	}

	const std::string &token_value = token.get_first();

	if (token_value == "True") {
		this->value = true;
	}
	else if (token_value == "False") {
		this->value = false;
	}
	else {
		throw LangError{
			token,
			"unknown boolean value (did you use 'True' and 'False'?)"
		};
	}
}


ValueHolder Boolean::copy() const {
	return ValueHolder{
		std::make_shared<Boolean>(dynamic_cast<const Boolean &>(*this))
	};
}


void Boolean::apply_value(const Value &value, nyan_op operation) {
	const Boolean &change = dynamic_cast<const Boolean &>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change.value; break;

	case nyan_op::UNION_ASSIGN:
		this->value |= change.value; break;

	case nyan_op::INTERSECT_ASSIGN:
		this->value &= change.value; break;

	default:
		throw Error{"unknown operation requested"};
	}
}


std::string Boolean::str() const {
	if (this->value) {
		return "True";
	}
	else {
		return "False";
	}
}


std::string Boolean::repr() const {
	return this->str();
}


size_t Boolean::hash() const {
	return std::hash<bool>{}(this->value);
}


bool Boolean::equals(const Value &other) const {
	auto &other_val = dynamic_cast<const Boolean &>(other);
	return this->value == other_val.value;
}


const std::unordered_set<nyan_op> &Boolean::allowed_operations(const Type &with_type) const {

	const static std::unordered_set<nyan_op> none_ops{
		nyan_op::ASSIGN,
	};

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	switch (with_type.get_primitive_type()) {
	case primitive_t::BOOLEAN:
		return ops;

	case primitive_t::NONE:
		return none_ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &Boolean::get_type() const {
	constexpr static BasicType type{
		primitive_t::BOOLEAN,
		composite_t::SINGLE,
	};

	return type;
}


} // namespace nyan
