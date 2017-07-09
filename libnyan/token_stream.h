// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "error.h"


namespace nyan {

/**
 * Python-yield like iterator for a token stream.
 * You can fetch the next value until nothing is left.
 *
 * The passed container is stored as reference only,
 * so it must be kept owned in the outside.
 */
class TokenStream {
public:
	using tok_t = Token;
	using container_t = std::vector<tok_t>;
	using iter_t = container_t::const_iterator;

	TokenStream(const container_t &container);

	~TokenStream();

	const tok_t *next();

	bool full() const;

	bool empty() const;

	/** really stupid variant of one-lookahead... */
	void reinsert(const tok_t *item);

protected:
	const std::vector<Token> &container;
	iter_t iter;
	const tok_t *reinserted;
};

} // namespace nyan
