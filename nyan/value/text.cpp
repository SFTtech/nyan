// Copyright 2016-2023 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "text.h"

#include <typeinfo>

#include "../compiler.h"
#include "../id_token.h"
#include "../lang_error.h"
#include "../token.h"
#include "../util.h"


namespace nyan {

Text::Text(const std::string &value) :
	value{value} {}


Text::Text(const IDToken &token) {
	if (unlikely(token.get_type() != token_type::STRING)) {
		throw LangError{
			token,
			"invalid value for text"};
	}

	// strip the quotes
	this->value = token.get_first().substr(1, token.get_first().size() - 2);
}


ValueHolder Text::copy() const {
	return {std::make_shared<Text>(*this)};
}


bool Text::apply_value(const Value &value, nyan_op operation) {
	const Text &change = dynamic_cast<const Text &>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change.value;
		break;

	case nyan_op::ADD_ASSIGN:
		this->value += change.value;
		break;

	default:
		throw InternalError{"unknown operation requested"};
	}

	return true;
}


std::string Text::str() const {
	return "\"" + this->value + "\"";
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

	switch (with_type.get_primitive_type()) {
	case primitive_t::TEXT:
		return ops;

	default:
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
