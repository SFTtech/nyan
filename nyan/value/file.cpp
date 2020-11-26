// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "file.h"

#include <typeinfo>

#include "../compiler.h"
#include "../lang_error.h"
#include "../id_token.h"
#include "../util.h"


namespace nyan {

Filename::Filename(const std::string &path)
	:
	path{path} {

	// TODO relative path resolution
}


Filename::Filename(const IDToken &token)
	:
	Filename{token.get_first()} {

	if (unlikely(token.get_type() != token_type::STRING)) {
		throw LangError{
			token,
			"invalid value for filename"
		};
	}
}


const std::string &Filename::get() const {
	return this->path;
}


ValueHolder Filename::copy() const {
	return {std::make_shared<Filename>(*this)};
}


void Filename::apply_value(const Value &value, nyan_op operation) {
	const Filename &change = dynamic_cast<const Filename &>(value);

	// TODO: relative path resolution

	switch (operation) {
	case nyan_op::ASSIGN:
		this->path = change.path; break;

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


const std::unordered_set<nyan_op> &Filename::allowed_operations(const Type &with_type) const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
	};

	if (with_type.get_primitive_type() == primitive_t::FILENAME) {
		return ops;
	}
	else {
		return no_nyan_ops;
	}
}


const BasicType &Filename::get_type() const {
	constexpr static BasicType type{
		primitive_t::FILENAME,
		composite_t::NONE,
	};

	return type;
}


} // namespace nyan
