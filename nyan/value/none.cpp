// Copyright 2020-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include <memory>

#include "none.h"


namespace nyan {

None::None() = default;


ValueHolder None::copy() const {
	throw InternalError("cannot copy None, hardcoded value should be referenced instead");
}


void None::apply_value(const Value &/**value*/, nyan_op /**operation*/) {
	throw InternalError("cannot apply to None: assign Value directly to member instead");
}


std::string None::str() const {
	return "None";
}


std::string None::repr() const {
	return this->str();
}


size_t None::hash() const {
	return std::hash<std::string>{}("None");
}


bool None::equals(const Value &other) const {
	// Check address equality because we only want identity
	return std::addressof(*this) == std::addressof(other);
}


const std::unordered_set<nyan_op> &None::allowed_operations(const Type &with_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
	};

	switch (with_type.get_primitive_type())
	{
	case primitive_t::BOOLEAN:
	case primitive_t::INT:
	case primitive_t::FLOAT:
	case primitive_t::TEXT:
	case primitive_t::FILENAME:
	case primitive_t::NONE:
	case primitive_t::CONTAINER:
		return ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &None::get_type() const {
	constexpr static BasicType type{
		primitive_t::NONE,
		composite_t::SINGLE,
	};

	return type;
}


} // namespace nyan
