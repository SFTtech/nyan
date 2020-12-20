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

    /**
     * Advance one step in the stream and get the pointer to
     * the token the streamnow points to.
     *
     * @return Next token in the stream.
     */
    const tok_t *next();

    /**
     * Check if the end of the stream has been reached, i.e. there
     * are no more tokens after the current token.
     *
     * @return true if there are no more tokens in the stream, else false.
     */
    bool full() const;

    /**
     * Check if there are tokens left in the stream.
     *
     * @return true if there are more tokens in the stream, else false.
     */
    bool empty() const;

    /**
     * Reinserts the token previously returned by next().
     */
    void reinsert_last();

protected:
    /**
     * List of tokens in the stream.
     */
    const container_t &container;

    /**
     * Iterator used for advancing/regressing in the stream.
     */
    container_t::const_iterator iter;
};

} // namespace nyan
