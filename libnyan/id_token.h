// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>
#include <vector>

#include "token.h"
#include "token_stream.h"


namespace nyan {


/**
 * Sometimes tokens, used as identifiers, can be concatenated by dots sometimes.
 * Used for e.g. namespace references, or ambiguous members of parent objects.
 * This multi-token groups those.
 */
class IDToken {
public:
	IDToken() = default;
	IDToken(const Token &first, TokenStream &tokens);

	std::string str() const;

	bool exists() const;

	token_type get_type() const;

	const Location &get_start_location() const;
	size_t get_length() const;

	const std::vector<Token> &get_components() const;
	const std::string &get_first() const;

protected:
	std::vector<Token> ids;
};


} // namespace nyan
