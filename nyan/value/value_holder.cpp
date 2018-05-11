// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

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


Value *ValueHolder::get_value() const {
	return this->value.get();
}


const std::shared_ptr<Value> &ValueHolder::get_ptr() const {
	return this->value;
}


void ValueHolder::clear() {
	this->value = nullptr;
}


bool ValueHolder::exists() const {
	return this->get_value() != nullptr;
}


Value &ValueHolder::operator *() const {
	return *this->get_value();
}


Value *ValueHolder::operator ->() const {
	return this->get_value();
}


bool ValueHolder::operator ==(const ValueHolder &other) const  {
	return (*this->get_value() == *other.get_value());
}


bool ValueHolder::operator !=(const ValueHolder &other) const {
	return (*this->get_value() != *other.get_value());
}


} // namespace nyan


namespace std {

size_t hash<nyan::ValueHolder>::operator ()(const nyan::ValueHolder &val) const {
	return hash<nyan::Value *>{}(val.get_value());
}

} // namespace std
