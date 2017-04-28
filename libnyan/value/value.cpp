// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value.h"

#include "../error.h"
#include "../member.h"
#include "../token.h"


namespace nyan {

Value::Value() {}

void Value::apply(const Member *change) {
	// extract the member's value,
	// this is just the data of the member,
	// no parent data is included.
	const Value *value = change->get_value_ptr();

	// TODO: cache usage: if the value has a cached value,
	//       stop the patch loop and just use this value.
	//       BUT this will fail if one is in a diamond?

	// TODO: this check is too strict. child classes are allowed as well.
	if (typeid(value) != typeid(this)) {
		throw TypeError{
			change->get_location(),
			"applied member which has incompatible type"
		};
	}

	this->apply_value(value, change->get_operation());
}

bool Value::operator ==(const Value &other) const {
	if (typeid(*this) != typeid(other)) {
		return false;
	}
	return this->equals(other);
}

bool Value::operator !=(const Value &other) const {
	return not (*this == other);
}

} // namespace nyan
