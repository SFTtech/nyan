// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>

#include "../lang_error.h"
#include "../token.h"


namespace nyan {

class File;

namespace lexer {
class Impl;
} // namespace lexer


class Lexer {
public:
	/**
	 * Create a lexer for the given file.
	 */
	Lexer(const std::shared_ptr<File> &file);
	virtual ~Lexer();

	// no moves and copies
	Lexer(Lexer &&other) = delete;
	Lexer(const Lexer &other) = delete;
	Lexer &operator=(Lexer &&other) = delete;
	Lexer &operator=(const Lexer &other) = delete;

	/**
	 * Return the next available token.
	 */
	Token get_next_token();

protected:
	/** Lexer internal implementation */
	std::unique_ptr<lexer::Impl> impl;
};


/**
 * Exception for lexer problems.
 */
class LexerError : public LangError {
public:
	LexerError(const Location &location, const std::string &msg);
};


} // namespace nyan
