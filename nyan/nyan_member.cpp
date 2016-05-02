// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_member.h"

namespace nyan {


NyanMember::NyanMember(NyanTypeContainer &&type, const NyanLocation &location)
	:
	type{std::move(type)},
	operation{nyan_op::INVALID},
	cached_value{nullptr},
	value{},
	location{location} {}


NyanMember::NyanMember(NyanTypeContainer &&type, nyan_op operation,
                       NyanValueContainer &&value,
                       const NyanLocation &location)
	:
	type{std::move(type)},
	operation{operation},
	cached_value{nullptr},
	value{std::move(value)},
	location{location} {

	// TODO check if type, operation and value can work together
}


NyanMember::NyanMember(NyanMember &&other)
	:
	type{std::move(other.type)},
	operation{other.operation},
	cached_value{std::move(other.cached_value)},
	value{std::move(other.value)},
	location{std::move(other.location)} {}


const NyanMember &NyanMember::operator =(NyanMember &&other) {
	this->type = std::move(other.type);
	this->operation = other.operation;
	this->cached_value = std::move(other.cached_value);
	this->value = std::move(other.value);
	this->location = std::move(other.location);
	return *this;
}


std::string NyanMember::str() const {
	return "TODO string representation of member";
}


NyanType *NyanMember::get_type() const {
	return this->type.get();
}


NyanValue *NyanMember::get_value() const {
	return this->value.get();
}


void NyanMember::set_value(NyanValueContainer &&val) {
	// replace the container
	this->value = std::move(val);
}


void NyanMember::set_value(std::unique_ptr<NyanValue> &&val) {
	this->value.set(std::move(val));
}


nyan_op NyanMember::get_operation() const {
	return this->operation;
}


void NyanMember::cache_save(std::unique_ptr<NyanValue> &&value) {
	this->cached_value = std::move(value);
}


NyanValue *NyanMember::cache_get() const {
	return this->cached_value.get();
}


void NyanMember::cache_reset() {
	this->cached_value.reset(nullptr);
}

const NyanLocation &NyanMember::get_location() const {
	return this->location;
}

} // namespace nyan
