// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "file.h"

#include <typeinfo>

#include "error.h"
#include "token.h"


namespace nyan {

Filename::Filename(const std::string &path)
	:
	path{path} {}


Filename::Filename(const Token &token)
	:
	Filename{token.get()} {}


std::unique_ptr<Value> Filename::copy() const {
	return std::make_unique<Filename>(dynamic_cast<const Filename &>(*this));
}


void Filename::apply_value(const Value *value, nyan_op operation) {
	const Filename *change = dynamic_cast<const Filename *>(value);

	// TODO: relative path resolution

	switch (operation) {
	case nyan_op::ASSIGN:
		this->path = change->path; break;

	default:
		throw Error{"unknown operation requested"};
	}
}


std::string Filename::str() const {
	return this->path;
}


std::string Filename::repr() const {
	return this->str();
}


size_t Filename::hash() const {
	return std::hash<std::string>{}(this->path);
}


bool Filename::equals(const Value &other) const {
	auto &other_val = dynamic_cast<const Filename &>(other);
	return this->path == other_val.path;
}


const std::unordered_set<nyan_op> &Filename::allowed_operations(nyan_basic_type value_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
	};

	if (value_type.primitive_type == nyan_primitive_type::TEXT) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


const nyan_basic_type &Filename::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::FILENAME,
		nyan_container_type::SINGLE,
	};

	return type;
}


} // namespace nyan
