// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>
#include <vector>

#include "token.h"
#include "token_stream.h"


namespace nyan {


/**
 * String tokens, used as identifiers, can be concatenated by dots sometimes.
 * Used for e.g. namespace references, or ambiguous members of parent objects.
 * This multi-token groups those.
 */
class IDToken {
public:
	IDToken() = default;
	IDToken(const Token &first, TokenStream &tokens);

	/**
	 * Get the string representation of this IDToken.
	 *
	 * @return String representation formatted in nyan language notation.
	 */
	std::string str() const;

	/**
	 * Check if this IDToken is empty.
	 *
	 * @return true if the IDToken has more than one ID, else false.
	 */
	bool exists() const;

	/**
	 * Get the type of the IDToken's content.
	 *
	 * @return Type of the first ID in this IDToken or token_type::INVALID if it doesn't exist.
	 */
	token_type get_type() const;

	/**
	 * Get the starting location of this IDToken in a file.
	 *
	 * @return Location of this IDToken.
	 */
	const Location &get_start_location() const;

	/**
	 * Get the character length of this IDToken.
	 *
	 * @return Length of this IDToken.
	 */
	size_t get_length() const;

	/**
	 * Get the list of IDs in this IDToken.
	 *
	 * @return A list of Tokens in this IDToken.
	 */
	const std::vector<Token> &get_components() const;

	/**
	 * Get the string representation of the first ID in this IDToken.
	 *
	 * @return String representation of the first ID formatted in nyan language notation.
	 */
	const std::string &get_first() const;

protected:
	/**
	 * List of IDs defining the IDToken.
	 */
	std::vector<Token> ids;
};


} // namespace nyan
