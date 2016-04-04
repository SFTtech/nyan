// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_member.h"

namespace nyan {

NyanMember::NyanMember()
	:
	operation{nyan_op::INVALID},
	cached_value{nullptr} {}


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


NyanOwningMember::NyanOwningMember(std::unique_ptr<NyanValue> &&value)
	:
	value{std::move(value)} {}


NyanValue *NyanOwningMember::get() const {
	return this->value.get();
}


NyanPtrMember::NyanPtrMember(NyanValue *value)
	:
	value{value} {}


NyanValue *NyanPtrMember::get() const {
	return this->value;
}

} // namespace nyan
