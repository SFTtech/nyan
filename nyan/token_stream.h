// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <vector>


namespace nyan {

class Token;

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

	TokenStream(const container_t &container);

	~TokenStream();

	const tok_t *next();

	bool full() const;

	bool empty() const;

	/**
	 * Reinserts the tokens previously returned by next in reverse order.
	 */
	void reinsert_last();

protected:
	const container_t &container;
	container_t::const_iterator iter;
};

} // namespace nyan
