// Copyright 2017-2018 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <queue>
#include <stack>

#include "bracket.h"

namespace nyan::lexer {

/// Interface with flex generated lexer.
class Impl {
public:

	explicit Impl(const std::shared_ptr<File> &file);

	~Impl();

	/** No copies. No moves. */
	Impl(const Impl &other) = delete;
	Impl(Impl&& other) = delete;
	const Impl &operator =(const Impl &other) = delete;
	Impl &&operator =(Impl &&other) = delete;

	/** Produce a token by reading the input. */
	Token generate_token();

/** @name FlexInterfaceMethods
 * Methods used by the flex generated lexer.
 */
///@{

	/** Advance the line position by match length. */
	void advance_linepos();

	/**
	 * Try to read `max_size` bytes to `buffer`.
	 * Return number of bytes read.
	 */
	int read_input(char *buffer, int max_size);

	/**
	 * Create a token with correct text position and value.
	 * Add the token to the queue.
	 */
	void token(token_type type);

	/** Tokenize error was encountered. */
	TokenizeError error(const std::string &msg);

	/** Emit line ending token for current position. */
	void endline();

	/** Generate indentation tokens based on given depth. */
	void handle_indent(int depth);

///@}

protected:

	/**
	 * Indentation enforcement in parens requires to track
	 * the open and closing parens `(<[{}]>)`.
	 */
	void track_brackets(token_type type, int token_start);

	/** Input file used for tokenization. */
	std::shared_ptr<File> file;

	/** String stream which is fed into the lexer. */
	std::istringstream input;

	/** Available tokens. */
	std::queue<Token> tokens;

	/** The indentation level of the previous line. */
	int previous_indent = 0;

	/** The bracket stack remembers current open positions of `(<[{}]>)`. */
	std::stack<Bracket> brackets;

	/**
	 * Set to true when a opening bracket was encountered.
	 * If it is true and the next token is a newline, the bracket is hanging.
	 * It will be set to false when the token after a opening
	 * bracket was processed.
	 */
	bool possibly_hanging = false;

	/**
	 * True when the indentation in brackets doesn't match.
	 * This is only the case when for a closing bracket.
	 */
	bool bracketcloseindent_expected = false;

	/** The default line positon at the very beginning of one line. */
	static constexpr int linepos_start = 0;

	/** Current position in a line. */
	int linepos = linepos_start;

	/** yyscan_t object: pointer to flex generated lexer */
	void *scanner{nullptr};
};

} // namespace nyan::lexer
