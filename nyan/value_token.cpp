// Copyright 2020-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#include "value_token.h"

#include <string>

#include "ast.h"
#include "compiler.h"
#include "error.h"
#include "location.h"

namespace nyan {


ValueToken::ValueToken(const IDToken &token)
	:
	container_type{composite_t::SINGLE} {

	this->tokens.push_back(token);
}


ValueToken::ValueToken(composite_t type,
                       std::vector<IDToken> &tokens)
	:
	tokens{tokens} {

	const static std::unordered_set<composite_t> container_types{
		composite_t::SET,
		composite_t::ORDEREDSET,
		composite_t::DICT
	};

	if (container_types.find(type) == container_types.end()) {
		throw InternalError{"unknown container value type"};
	}

	this->container_type = type;
}


std::string ValueToken::str() const {
	switch (this->container_type) {
	case composite_t::SINGLE:
	case composite_t::SET:
	case composite_t::ORDEREDSET:
		return this->tokens.at(0).str();

	case composite_t::DICT:
		return this->tokens.at(0).str() + ": " + this->tokens.at(1).str();

	default:
		throw InternalError{"unknown container value type"};
	};
}


bool ValueToken::exists() const {
	return this->tokens.size() > 0;
}


bool ValueToken::is_none() const {
	if (this->tokens.size() != 1) {
		return false;
	}

	IDToken id_token = this->tokens[0];
	if (id_token.get_components().size() != 1) {
		return false;
	}

	return id_token.get_components()[0].get() == "None";
}

const Location &ValueToken::get_start_location() const {
	if (unlikely(not this->exists())) {
		throw InternalError{"this ValueToken doesn't exist, but you queried its location"};
	}

	return this->tokens.at(0).get_start_location();
}


size_t ValueToken::get_length() const {
	if (not this->exists()) {
		return 0;
	}

	switch (this->container_type) {
	case composite_t::SINGLE:
	case composite_t::SET:
	case composite_t::ORDEREDSET:
		return this->tokens.at(0).get_length();

	case composite_t::DICT:
		// key token length + value token length + separating ": " length
		return this->tokens.at(0).get_length() +
		this->tokens.at(1).get_length() + 2;

	default:
		throw InternalError{"unknown container value type"};
	};
}


const std::vector<IDToken> &ValueToken::get_value() const {
	return this->tokens;
}


const composite_t &ValueToken::get_container_type() const {
	return this->container_type;
}


} // namespace nyan
