// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value_holder.h"

#include "value.h"


namespace nyan {

ValueHolder::ValueHolder() = default;


ValueHolder::ValueHolder(std::shared_ptr<Value> &&value)
	:
	value{std::move(value)} {}


ValueHolder::ValueHolder(const std::shared_ptr<Value> &value)
	:
	value{value} {}


ValueHolder &ValueHolder::operator =(const std::shared_ptr<Value> &value) {
	this->value = value;
	return *this;
}


const std::shared_ptr<Value> &ValueHolder::get_ptr() const {
	return this->value;
}


bool ValueHolder::exists() const {
	return this->value.get() != nullptr;
}


Value &ValueHolder::operator *() const {
	return *this->value;
}


Value *ValueHolder::operator ->() const {
	return this->value.get();
}


bool ValueHolder::operator ==(const ValueHolder &other) const  {
	return (*this->value == *other.value);
}


bool ValueHolder::operator !=(const ValueHolder &other) const {
	return (*this->value != *other.value);
}


} // namespace nyan


namespace std {

size_t hash<nyan::ValueHolder>::operator ()(const nyan::ValueHolder &val) const {
	return hash<nyan::Value>{}(*val);
}

} // namespace std
