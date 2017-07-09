// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value_holder.h"


namespace nyan {

ValueHolder::ValueHolder() {}


ValueHolder::ValueHolder(std::shared_ptr<Value> &&value)
	:
	value{std::move(value)} {}


ValueHolder::ValueHolder(const std::shared_ptr<Value> &value)
	:
	value{value} {}



Value *ValueHolder::get_value() const {
	return this->value.get();
}


void ValueHolder::clear() {
	this->value = nullptr;
}


bool ValueHolder::exists() const {
	return this->get_value() != nullptr;
}

} // namespace nyan


namespace std {

size_t hash<nyan::ValueHolder>::operator ()(const nyan::ValueHolder &val) const {
	return hash<nyan::ValueWrapper>{}(static_cast<const nyan::ValueWrapper &>(val));
}

} // namespace std
