// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_error.h"
#include "nyan_member.h"
#include "nyan_token.h"
#include "nyan_value.h"


namespace nyan {

NyanValue::NyanValue() {}

void NyanValue::apply(const NyanMember *change) {
	// extract the member's value,
	// this is just the data of the member,
	// no parent data is included.
	const NyanValue *value = change->get_value_ptr();

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

bool NyanValue::operator ==(const NyanValue &other) const {
	if (typeid(*this) != typeid(other)) {
		return false;
	}
	return this->equals(other);
}

bool NyanValue::operator !=(const NyanValue &other) const {
	return not (*this == other);
}

} // namespace nyan
