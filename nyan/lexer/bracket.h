// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include "../token.h"


namespace nyan::lexer {


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

} // namespace nyan::lexer
