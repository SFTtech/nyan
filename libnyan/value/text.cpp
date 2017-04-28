// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "text.h"

#include <typeinfo>

#include "../error.h"
#include "../token.h"


namespace nyan {

Text::Text(const std::string &value)
	:
	value{value} {}


Text::Text(const Token &token)
	:
	Text{token.get()} {}


std::unique_ptr<Value> Text::copy() const {
	return std::make_unique<Text>(dynamic_cast<const Text &>(*this));
}


void Text::apply_value(const Value *value, nyan_op operation) {
	const Text *change = dynamic_cast<const Text *>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change->value; break;

	case nyan_op::ADD_ASSIGN:
		this->value += change->value; break;

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


const std::unordered_set<nyan_op> &Text::allowed_operations(nyan_basic_type value_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
	};

	if (value_type.primitive_type == nyan_primitive_type::TEXT) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


const nyan_basic_type &Text::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::TEXT,
		nyan_container_type::SINGLE,
	};

	return type;
}


} // namespace nyan
