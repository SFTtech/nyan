// Copyright 2016-2025 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "token_stream.h"

#include <iostream>

#include "token.h"


namespace nyan {

TokenStream::TokenStream(const TokenStream::container_t &container) :
	container{container},
	iter{std::begin(container)} {}


TokenStream::~TokenStream() = default;


const TokenStream::tok_t *TokenStream::next() {
	const tok_t *ret;

	if (not this->full()) {
		throw InternalError{"requested item from empty list"};
	}

	ret = &(*this->iter);

	// std::cout << "tok: " << ret->str() << std::endl;

	this->iter = std::next(this->iter);
	return ret;
}


bool TokenStream::full() const {
	return this->iter != std::end(this->container);
}


bool TokenStream::empty() const {
	return not this->full();
}


void TokenStream::reinsert_last() {
	if (this->iter == std::begin(this->container)) {
		throw InternalError{"requested reinsert of unavailable token"};
	}

	this->iter = std::prev(this->iter);
}

} // namespace nyan
