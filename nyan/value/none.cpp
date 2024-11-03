// Copyright 2020-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "none.h"

#include <memory>

namespace nyan {


// global none value
std::shared_ptr<None> None::value = std::make_shared<None>();


None::None() = default;


ValueHolder None::copy() const {
	return ValueHolder{None::value};
}


bool None::apply_value(const Value & /**value*/, nyan_op /**operation*/) {
	throw InternalError{"None can't get an applied value - assign Value directly to member instead"};
}


std::string None::str() const {
	return "None";
}


std::string None::repr() const {
	return this->str();
}


size_t None::hash() const {
	return std::hash<std::string>{}("nyan_None");
}


bool None::equals(const Value & /*other*/) const {
	// none always equals none,
	// and `other` is ensured to be none by the `Value::operator==` check,
	// which then calls this `equals` member method.
	return true;
}


const std::unordered_set<nyan_op> &None::allowed_operations(const Type &with_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
	};

	// None can only be assigned to types with optional modifier flag
	if (not with_type.has_modifier(modifier_t::OPTIONAL)) {
		return no_nyan_ops;
	}

	return ops;
}


const BasicType &None::get_type() const {
	constexpr static BasicType type{
		primitive_t::NONE,
		composite_t::SINGLE,
	};

	return type;
}


} // namespace nyan
