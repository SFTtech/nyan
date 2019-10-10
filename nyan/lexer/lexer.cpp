// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "lexer.h"

#include "impl.h"


namespace nyan {

Lexer::Lexer(const std::shared_ptr<File> &file)
	:
	impl{std::make_unique<lexer::Impl>(file)} {

}

Lexer::~Lexer() = default;

// Return the token generated from the implementation.
Token Lexer::get_next_token() {
	return this->impl->generate_token();
}


LexerError::LexerError(const Location &location,
                       const std::string &msg)
	:
	LangError{location, msg} {}

} // namespace nyan
