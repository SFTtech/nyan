// Copyright 2020-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
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

	/**
	 * Simple constructor for a single value that is not in a container.
	 */
	ValueToken(const IDToken &token);

	/**
	 * Constructor for value tokens in a container.
	 */
	ValueToken(composite_t type,
	           std::vector<IDToken> &tokens);

	/**
	 * Get the string representation of this ValueToken.
	 *
	 * @return String representation formatted in nyan language notation.
	 */
	std::string str() const;

	/**
	 * Check if this ValueToken is empty.
	 *
	 * @return true if the ValueToken has more than one IDToken, else false.
	 */
	bool exists() const;

	/**
	 * Checks if this ValueToken contains the None value.
	 *
	 * @return true if this ValueToken stores None, else false.
	 */
	bool is_none() const;

	/**
	 * Get the starting location of this ValueToken in a file.
	 *
	 * @return Location of this ValueToken.
	 */
	const Location &get_start_location() const;

	/**
	 * Get the character length of this ValueToken.
	 *
	 * @return Length of this ValueToken.
	 */
	size_t get_length() const;

	/**
	 * Get the list of IDTokens in this ValueToken.
	 *
	 * @return List of IDTokens in this ValueToken.
	 */
	const std::vector<IDToken> &get_value() const;

protected:
	/**
	 * Get the type of container that this ValueToken is stored in.
	 *
	 * @return Container type of this ValueToken.
	 */
	const composite_t &get_container_type() const;

	/**
	 * Container type where the value is tored in.
	 */
	composite_t container_type;

	/**
	 * Components in the token.
	 */
	std::vector<IDToken> tokens;
};


} // namespace nyan
