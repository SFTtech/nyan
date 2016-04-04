// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_error.h"
#include "nyan_member.h"
#include "nyan_value.h"

#include <typeinfo>
#include <sstream>

namespace nyan {

//// Base class //////////////////////////////////
NyanValue::NyanValue() {}

bool NyanValue::apply(const NyanMember *change) {
	bool parents_included = false;
	const NyanValue *value = change->cache_get();

	if (value != nullptr) {
		parents_included = true;
	} else {
		value = change->get();
	}

	if (typeid(value) != typeid(*this)) {
		throw TypeError{"applied member which has value of wrong type"};
	}

	this->apply_value(value, change->get_operation());

	return parents_included;
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

//////////////////////////////////////////////////


//// Text ////////////////////////////////////////
NyanText::NyanText(const std::string &value)
	:
	value{value} {}

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

size_t NyanText::hash() const {
	return std::hash<std::string>{}(this->value);
}

bool NyanText::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanText &>(other);
	return this->value == other_val.value;
}
//////////////////////////////////////////////////

//// Int /////////////////////////////////////////
NyanInt::NyanInt(int64_t value)
	:
	value{value} {}

std::unique_ptr<NyanValue> NyanInt::copy() const {
	return std::make_unique<NyanInt>(dynamic_cast<const NyanInt &>(*this));
}

void NyanInt::apply_value(const NyanValue *value, nyan_op operation) {
	const NyanInt *change = dynamic_cast<const NyanInt *>(value);

	switch (operation) {
	case nyan_op::ASSIGN:
		this->value = change->value; break;

	case nyan_op::ADD_ASSIGN:
		this->value += change->value; break;

	case nyan_op::SUBTRACT_ASSIGN:
		this->value -= change->value; break;

	case nyan_op::MULTIPLY_ASSIGN:
		this->value *= change->value; break;

	case nyan_op::DIVIDE_ASSIGN:
		this->value /= change->value; break;

	default:
		throw NyanError{"unknown operation requested"};
	}
}

std::string NyanInt::str() const {
	std::ostringstream builder;
	builder << this->value;
	return builder.str();
}

size_t NyanInt::hash() const {
	return this->value;
}

bool NyanInt::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanInt &>(other);
	return this->value == other_val.value;
}
//////////////////////////////////////////////////

} // namespace nyan
