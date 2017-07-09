// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value_wrapper.h"

#include "value.h"


namespace nyan {

ValueWrapper::ValueWrapper() {}


ValueWrapper::~ValueWrapper() {}


Value &ValueWrapper::operator *() const {
	return *this->get_value();
}


Value *ValueWrapper::operator ->() const {
	return this->get_value();
}


bool ValueWrapper::operator ==(const ValueWrapper &other) const {
	return (*this->get_value() == *other.get_value());
}


bool ValueWrapper::operator !=(const ValueWrapper &other) const {
	return (*this->get_value() != *other.get_value());
}

} // namespace nyan


namespace std {

size_t hash<nyan::ValueWrapper>::operator ()(const nyan::ValueWrapper &val) const {
	return hash<nyan::Value *>{}(val.get_value());
}

} // namespace std
