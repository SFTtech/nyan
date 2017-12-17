// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include "../error.h"
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
	Lexer &operator =(Lexer &&other) = delete;
	Lexer &operator =(const Lexer &other) = delete;

	/**
	 * Return the next available token.
	 */
	Token get_next_token();

protected:

	class Bracket {
	public:
		Bracket(token_type type, int indent);

		/** This bracket is directly followed by a newline. */
		void doesnt_hang(int hanging_indent);

		/** Was this bracket not directly followed by a newline? */
		bool is_hanging() const;

		/** Does this bracket match the given token type? */
		bool matches(token_type type) const;

		/** Return the expected content indentation level. */
		int get_content_indent() const;

		/** Return the expected closing bracket indent level. */
		std::string get_closing_indent() const;

		/** Check if the closing indent is ok. */
		bool closing_indent_ok(int indent) const;

		/** Return the visual representation of the expected bracket */
		const char *matching_type_str() const;

		/** convert the token type to bracket type */
		static bracket_type to_type(token_type token);

	protected:
		/** expected closing bracket type */
		token_type expected_match() const;

		/**
		 * Indentation level of the line this bracket was in.
		 */
		int indentation;

		/**
		 * Type of this opening bracket.
		 */
		bracket_type type;

		/**
		 * true if the indentation mode is "hanging",
		 * that is, if a continuation must happen at the
		 * indent level of this opening bracket.
		 * The expected indentation level of contained content is stored
		 * in the `indentation` member.
		 */
		bool hanging;
	};

	/**
	 * Internal implementation of the lexer: incomplete type here.
	 */
	friend lexer::Impl;

	/**
	 * Indentation enforcement in parens requires to track
	 * the open and closing parens `(<[{}]>)`.
	 */
	void track_brackets(token_type type, int token_start);

	/**
	 * Measure the indentation of a line,
	 * Generate indentation tokens.
	 * Return the difference to the previous indent.
	 */
	void handle_indent(const char *line);

	/**
	 * The indentation stack remembers the levels of indent.
	 */
	std::vector<int> indent_stack;

	/**
	 * The bracket stack remembers current open positions
	 * of `(<[{}]>)`.
	 */
	std::vector<Bracket> bracket_stack;

	/**
	 * Set to true when a opening bracket was encountered.
	 * If it is true and the next token is a newline,
	 * the bracket is hanging.
	 * It will be set to false when the token after a opening
	 * bracket was processed.
	 */
	bool possibly_hanging = false;

	/**
	 * True when the indentation in brackets doesn't match.
	 * This is only the case when for a closing bracket.
	 */
	bool bracketcloseindent_expected = false;

	/**
	 * Input file used for tokenization.
	 */
	std::shared_ptr<File> file;

	/**
	 * String stream which is fed into the lexer.
	 */
	std::istringstream input;

	/**
	 * Available tokens.
	 */
	std::queue<Token> tokens;

	/**
	 * Indicates whether the tokenization has reached the end.
	 */
	bool finished = false;

	/** Lexer internal implementation */
	std::unique_ptr<lexer::Impl> impl;
};


/**
 * Exception for lexer problems.
 */
class LexerError : public FileError {
public:
	LexerError(const Location &location, const std::string &msg);
};


} // namespace nyan
