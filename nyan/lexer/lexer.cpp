// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "lexer.h"

#include "../file.h"
#include "impl.h"


namespace nyan {

Lexer::Lexer(const std::shared_ptr<File> &file)
	:
	file{file},
	input{file->get_content()},
	impl{std::make_unique<lexer::Impl>(this)} {

	// The base indentation is zero of course.
	this->indent_stack.push_back(0);
}

Lexer::~Lexer() = default;

/*
 * Generate tokens until the queue has on available to return.
 * Return tokens from the queue until it's empty.
 */
Token Lexer::get_next_token() {
	if (this->finished) {
		throw this->impl->error("requested token but at EOF");
	}

	if (this->tokens.empty()) {
		this->impl->generate_token();
	}

	while (not this->tokens.empty()) {
		auto ret = this->tokens.front();
		this->tokens.pop();
		return ret;
	}

	// if generate_token did not generate a token:
	throw this->impl->error("internal error.");
}


Lexer::Bracket::Bracket(token_type ttype, int indent)
	:
	indentation{indent},
	type{this->to_type(ttype)},
	hanging{true} {}


void Lexer::Bracket::doesnt_hang(int new_indent) {
	this->hanging = false;
	this->indentation = new_indent;
}


bool Lexer::Bracket::is_hanging() const {
	return this->hanging;
}


bool Lexer::Bracket::matches(token_type type) const {
	return type == this->expected_match();
}


int Lexer::Bracket::get_content_indent() const {
	if (this->is_hanging()) {
		// hanging brackets store their expected indent level
		return this->indentation;
	}
	else {
		// wrapped brackets store their base indent level
		return this->indentation + SPACES_PER_INDENT;
	}
}


bool Lexer::Bracket::closing_indent_ok(int indent) const {
	if (this->is_hanging()) {
		// hanging indent requires the closing bracket to be
		// closed after the opening bracket column
		return this->indentation <= indent;
	}
	else {
		return this->indentation == indent;
	}
}


std::string Lexer::Bracket::get_closing_indent() const {
	if (this->is_hanging()) {
		std::ostringstream builder;
		builder << "at least "
		        << this->indentation;
		return builder.str();
	}
	else {
		return std::to_string(this->indentation);
	}
}


const char *Lexer::Bracket::matching_type_str() const {
	return token_type_str(this->expected_match());
}


bracket_type Lexer::Bracket::to_type(token_type token) {
	switch (token) {
	case token_type::LPAREN:
	case token_type::RPAREN:
		return bracket_type::PAREN;
	case token_type::LANGLE:
	case token_type::RANGLE:
		return bracket_type::ANGLE;
	case token_type::LBRACKET:
	case token_type::RBRACKET:
		return bracket_type::BRACKET;
	case token_type::LBRACE:
	case token_type::RBRACE:
		return bracket_type::BRACE;
	default:
		throw InternalError{"tried to convert non-bracket token to bracket"};
	}
}


token_type Lexer::Bracket::expected_match() const {
	token_type expected;

	switch (this->type) {
	case bracket_type::PAREN:
		expected = token_type::RPAREN; break;
	case bracket_type::ANGLE:
		expected = token_type::RANGLE; break;
	case bracket_type::BRACKET:
		expected = token_type::RBRACKET; break;
	case bracket_type::BRACE:
		expected = token_type::RBRACE; break;
	default:
		throw InternalError{"unknown bracket type"};
	}

	return expected;
}

/*
 * Remember where the current open bracket is
 * so that the indentation can check if the depth is correct.
 */
void Lexer::track_brackets(token_type type, int token_start) {

	// opening brackets
	if (type == token_type::LPAREN or
	    type == token_type::LANGLE or
	    type == token_type::LBRACKET or
	    type == token_type::LBRACE) {

		// Track bracket type and indentation.
		// The position after the ( is exactly the expected indent
		// for hanging brackets.
		this->bracket_stack.push_back(
			Bracket{
				type,
				token_start + 1
			}
		);

		this->possibly_hanging = true;
		return;
	}
	// closing brackets
	else if (type == token_type::RPAREN or
	         type == token_type::RANGLE or
	         type == token_type::RBRACKET or
	         type == token_type::RBRACE) {

		if (this->bracket_stack.empty()) {
			throw this->impl->error("unexpected closing bracket, "
			                  "as no opening one is known");
		}

		Bracket &matching_open_bracket = this->bracket_stack.back();

		// test if bracket actually matches
		if (not matching_open_bracket.matches(type)) {
			std::ostringstream builder;
			builder << "non-matching bracket: expected '"
			        << matching_open_bracket.matching_type_str()
			        << "' but got '" << token_type_str(type) << "'";
			throw this->impl->error(builder.str());
		}

		if (not matching_open_bracket.closing_indent_ok(token_start)) {
			std::ostringstream builder;
			builder << "wrong indentation of bracket: expected "
			        << matching_open_bracket.get_closing_indent()
			        << " indentation spaces (it is currently at "
			        << token_start << " spaces)";
			throw this->impl->error(builder.str());
		}

		this->bracketcloseindent_expected = false;
		this->bracket_stack.pop_back();
	}
	// newline directly after opening bracket
	// means regular indentation has to follow
	// and the bracket pair doesn't hang.
	else if (not this->bracket_stack.empty() and
	         this->possibly_hanging and
	         type == token_type::ENDLINE) {

		// the bracket is followed by a newline directly,
		// thus is not hanging.
		this->bracket_stack.back().doesnt_hang(
			this->indent_stack.back()
		);
	}
	else if (not this->bracket_stack.empty() and
	         this->bracketcloseindent_expected) {
		std::ostringstream builder;
		builder << ("expected closing bracket or content "
		            "at indentation with ")
		        << this->bracket_stack.back().get_content_indent()
		        << " spaces (you start at " << token_start << " spaces)";
		throw this->impl->error(builder.str());
	}

	this->possibly_hanging = false;
}

/*
 * measure the indentation of a line
 */
void Lexer::handle_indent(const char *line) {

	// measure current indent
	int depth = 0;
	while (*line == ' ') {
		depth++;
		line++;
	}

	// Indentation depth of the last line
	int last_depth = 0;

	// regular indent is enforced when not in a bracket pair
	if (this->bracket_stack.empty()) {
		if ((depth % SPACES_PER_INDENT) > 0) {
			std::ostringstream builder;
			builder << "indentation requires exactly "
					<< SPACES_PER_INDENT
					<< " spaces per level";
			throw this->impl->error(builder.str());
		}
	}
	// we're in a pair of brackets,
	// there the indentation is way funnier.
	else {

		// check if the content indentation is correct.
		int expected = this->bracket_stack.back().get_content_indent();
		if (depth != expected) {
			// if the expected depth is not correct,
			// then the only thing that is allowed is
			// the closing bracket.
			// the check will be done for the next token in
			// `track_brackets`.
			this->bracketcloseindent_expected = true;
		}

		// don't need to track the indent stack,
		// this is done in the bracket tracking now.
		return;
	}

	if (not this->indent_stack.empty()) {
		int previous_depth = last_depth = this->indent_stack.back();

		if (depth == previous_depth) {
			// same indent level
		}
		else if (depth < previous_depth) {
			// current line is further left than the previous one

			// pop indent stack until current level is reached
			while (previous_depth > depth) {
				int delta = previous_depth - depth;
				while (delta > 0) {
					delta -= SPACES_PER_INDENT;
					this->impl->token(token_type::DEDENT);
				}

				this->indent_stack.pop_back();
				previous_depth = this->indent_stack.back();
			}
		}
		else {
			// new indent level
			this->indent_stack.push_back(depth);
			int delta = depth - last_depth;
			while (delta > 0) {
				delta -= SPACES_PER_INDENT;
				this->impl->token(token_type::INDENT);
			}
		}
	}
	else {
		throw InternalError{"indentation stack ran empty!?!?"};
	}
}


LexerError::LexerError(const Location &location,
                       const std::string &msg)
	:
	FileError{location, msg} {}

} // namespace nyan
