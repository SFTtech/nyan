// Copyright 2020-2029 the nyan authors, LGPLv3+. See copying.md for legal info.
#include "value_token.h"

#include <string>

#include "ast.h"
#include "compiler.h"
#include "error.h"
#include "location.h"

namespace nyan {


ValueToken::ValueToken(container_t type,
                       std::vector<IDToken> tokens) 
    :
	container_type{type},
	tokens{tokens} {

}

std::string ValueToken::str() const {
	switch (this->container_type) {
    case container_t::SINGLE:
    case container_t::SET:
    case container_t::ORDEREDSET:
        return this->tokens.at(0).str();

    case container_t::DICT:
        return this->tokens.at(0).str() + ": " + this->tokens.at(1).str();

    default:
        throw InternalError{"unknown container value type"};
    };
}

bool ValueToken::exists() const {
	return this->tokens.size() > 0;
}

const Location &ValueToken::get_start_location() const {
	if (unlikely(not this->exists())) {
		throw InternalError{
			"this ValueToken doesn't exist, but you queried its location"
		};
	}

	return this->tokens.at(0).get_start_location();
}

size_t ValueToken::get_length() const {
	if (not this->exists()) {
		return 0;
	}

	switch (this->container_type) {
    case container_t::SINGLE:
    case container_t::SET:
    case container_t::ORDEREDSET:
        return this->tokens.at(0).get_length();

    case container_t::DICT:
        // key length + value length + ": " separator length
        return this->tokens.at(0).get_length() +
               this->tokens.at(1).get_length() + 2;

    default:
        throw InternalError{"unknown container value type"};
    };
}

const std::vector<IDToken> &ValueToken::get_value() const {
	return this->tokens;
}

const container_t &ValueToken::get_container_type() const {
	return this->container_type;
}


} // namespace nyan
