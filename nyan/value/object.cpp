// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object.h"

#include <typeinfo>

#include "../error.h"
#include "../token.h"


namespace nyan {

ObjectValue::ObjectValue(const fqon_t &name)
	:
	name{name} {}


ValueHolder ObjectValue::copy() const {
	return {std::make_shared<ObjectValue>(*this)};
}


void ObjectValue::apply_value(const Value &value, nyan_op operation) {
	const ObjectValue &change = dynamic_cast<const ObjectValue &>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->name = change.name; break;

	default:
		throw Error{"unknown operation requested"};
	}
}


std::string ObjectValue::str() const {
	return this->name;
}


std::string ObjectValue::repr() const {
	return this->str();
}


size_t ObjectValue::hash() const {
	return std::hash<fqon_t>{}(this->name);
}


const fqon_t &ObjectValue::get() const {
	return this->name;
}


bool ObjectValue::equals(const Value &other) const {
	auto &other_val = dynamic_cast<const ObjectValue &>(other);
	return this->name == other_val.name;
}


const std::unordered_set<nyan_op> &ObjectValue::allowed_operations(const Type &with_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
	};

	switch (with_type.get_primitive_type()) {
	case primitive_t::OBJECT:
	case primitive_t::NONE:
		return ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &ObjectValue::get_type() const {
	constexpr static BasicType type{
		primitive_t::OBJECT,
		composite_t::SINGLE,
	};

	return type;
}


} // namespace nyan
