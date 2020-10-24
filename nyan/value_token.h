// Copyright 2020-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>
#include <vector>

#include "token.h"
#include "type.h"


namespace nyan {


/**
 * Stores a value. Values can consist of multiple IDTokens.
 */
class ValueToken {
public:
	ValueToken() = default;
	ValueToken(container_t type,
               std::vector<IDToken> tokens);

	std::string str() const;

	bool exists() const;

	const Location &get_start_location() const;
	size_t get_length() const;

	const std::vector<IDToken> &get_value() const;
	const container_t &get_container_type() const;

protected:
	container_t container_type;

	std::vector<IDToken> tokens;
};


} // namespace nyan
