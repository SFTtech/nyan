// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "token_stream.h"

#include "token.h"


namespace nyan {

TokenStream::TokenStream(const TokenStream::container_t &container)
	:
	container{container},
	iter{std::begin(container)},
	reinserted{nullptr} {}


TokenStream::~TokenStream() {}


const TokenStream::tok_t *TokenStream::next() {
	const tok_t *ret;
	if (this->reinserted != nullptr) {
		ret = this->reinserted;
		this->reinserted = nullptr;
		return ret;
	}

	if (not this->full()) {
		throw InternalError{"requested item from empty list"};
	}

	ret = &(*this->iter);

	//std::cout << "tok: " << ret->str() << std::endl;

	this->iter = std::next(this->iter);
	return ret;
}


bool TokenStream::full() const {
	return this->iter != std::end(this->container);
}


bool TokenStream::empty() const {
	return not this->full();
}


void TokenStream::reinsert(const tok_t *item) {
	this->reinserted = item;
}

} // namespace nyan
