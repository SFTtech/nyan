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

    /**
     * Simple constructor for a single value that is not in a container.
     */
	ValueToken(const IDToken &token);

    /**
     * Constructor for value tokens in a container.
     */
	ValueToken(composite_t type,
               std::vector<IDToken> &tokens);

	std::string str() const;

	bool exists() const;

	/**
	 * Checks if this value contains the None value.
	 */
	bool is_None() const;

	const Location &get_start_location() const;
	size_t get_length() const;

	const std::vector<IDToken> &get_value() const;
	const composite_t &get_container_type() const;

protected:
	composite_t container_type;

	std::vector<IDToken> tokens;
};


} // namespace nyan
