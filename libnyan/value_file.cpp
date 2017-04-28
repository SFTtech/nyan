// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value_file.h"

#include <typeinfo>

#include "error.h"
#include "token.h"


namespace nyan {

NyanFilename::NyanFilename(const std::string &path)
	:
	path{path} {}


NyanFilename::NyanFilename(const NyanToken &token)
	:
	NyanFilename{token.get()} {}


std::unique_ptr<NyanValue> NyanFilename::copy() const {
	return std::make_unique<NyanFilename>(dynamic_cast<const NyanFilename &>(*this));
}


void NyanFilename::apply_value(const NyanValue *value, nyan_op operation) {
	const NyanFilename *change = dynamic_cast<const NyanFilename *>(value);

	// TODO: relative path resolution

	switch (operation) {
	case nyan_op::ASSIGN:
		this->path = change->path; break;

	default:
		throw NyanError{"unknown operation requested"};
	}
}


std::string NyanFilename::str() const {
	return this->path;
}


std::string NyanFilename::repr() const {
	return this->str();
}


size_t NyanFilename::hash() const {
	return std::hash<std::string>{}(this->path);
}


bool NyanFilename::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanFilename &>(other);
	return this->path == other_val.path;
}


const std::unordered_set<nyan_op> &NyanFilename::allowed_operations(nyan_basic_type value_type) const {
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


const nyan_basic_type &NyanFilename::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::FILENAME,
		nyan_container_type::SINGLE,
	};

	return type;
}


} // namespace nyan
