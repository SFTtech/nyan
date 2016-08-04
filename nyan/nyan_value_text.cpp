// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_text.h"

#include <typeinfo>

#include "nyan_error.h"
#include "nyan_token.h"


namespace nyan {

NyanText::NyanText(const std::string &value)
	:
	value{value} {}


NyanText::NyanText(const NyanToken &token)
	:
	NyanText{token.get()} {}


std::unique_ptr<NyanValue> NyanText::copy() const {
	return std::make_unique<NyanText>(dynamic_cast<const NyanText &>(*this));
}


void NyanText::apply_value(const NyanValue *value, nyan_op operation) {
	const NyanText *change = dynamic_cast<const NyanText *>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change->value; break;

	case nyan_op::ADD_ASSIGN:
		this->value += change->value; break;

	default:
		throw NyanError{"unknown operation requested"};
	}
}


std::string NyanText::str() const {
	return this->value;
}


std::string NyanText::repr() const {
	return this->str();
}


size_t NyanText::hash() const {
	return std::hash<std::string>{}(this->value);
}


bool NyanText::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanText &>(other);
	return this->value == other_val.value;
}


const std::unordered_set<nyan_op> &NyanText::allowed_operations(nyan_basic_type value_type) const {
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


const nyan_basic_type &NyanText::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::TEXT,
		nyan_container_type::SINGLE,
	};

	return type;
}


} // namespace nyan
