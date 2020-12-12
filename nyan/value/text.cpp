// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "text.h"

#include <typeinfo>

#include "../compiler.h"
#include "../lang_error.h"
#include "../id_token.h"
#include "../util.h"
#include "../token.h"


namespace nyan {

Text::Text(const std::string &value)
	:
	value{value} {}


Text::Text(const IDToken &token)
	:
	Text{token.get_first()} {

	if (unlikely(token.get_type() != token_type::STRING)) {
		throw LangError{
			token,
			"invalid value for text"
		};
	}
}


ValueHolder Text::copy() const {
	return {std::make_shared<Text>(*this)};
}


void Text::apply_value(const Value &value, nyan_op operation) {
	const Text &change = dynamic_cast<const Text &>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change.value; break;

	case nyan_op::ADD_ASSIGN:
		this->value += change.value; break;

	default:
		throw Error{"unknown operation requested"};
	}
}


std::string Text::str() const {
	return this->value;
}


std::string Text::repr() const {
	return this->str();
}


size_t Text::hash() const {
	return std::hash<std::string>{}(this->value);
}


bool Text::equals(const Value &other) const {
	auto &other_val = dynamic_cast<const Text &>(other);
	return this->value == other_val.value;
}


const std::unordered_set<nyan_op> &Text::allowed_operations(const Type &with_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
	};

	if (with_type.get_primitive_type() == primitive_t::TEXT) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


const BasicType &Text::get_type() const {
	constexpr static BasicType type{
		primitive_t::TEXT,
		composite_t::SINGLE,
	};

	return type;
}


} // namespace nyan
