// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_lexer.h"

#include <sstream>

#include "nyan_file.h"

namespace nyan {

NyanLexer::NyanLexer(const NyanFile &file)
	:
	NyanFlexLexer{},
	file{file},
	input{file.get()},
	linepos{linepos_start},
	finished{false} {

	// set the input stream in the flex class
	this->switch_streams(&this->input, nullptr);

	// The base indentation is zero of course.
	this->indent_stack.push_back(0);
}

/*
 * Generate tokens until the queue has on available to return.
 * Return tokens from the queue until it's empty.
 */
NyanToken NyanLexer::get_next_token() {
	if (this->finished) {
		throw this->error("requested token but at EOF");
	}
	if (this->tokens.empty()) {
		this->generate_token();
	}
	while (not this->tokens.empty()) {
		auto ret = this->tokens.front();
		this->tokens.pop();
		return ret;
	}

	// if generate_token did not generate a token:
	throw this->error("internal error.");
}

/*
 * Fetch the current lexer state variables and create a token.
 */
void NyanLexer::token(token_type type) {
	if (token_needs_payload(type)) {
		this->tokens.push(NyanToken{
			this->file,
			this->yylineno,
			this->linepos - this->yyleng,
			type,
			this->yytext
		});
	}
	else {
		this->tokens.push(NyanToken{
			this->file,
			this->yylineno,
			this->linepos - this->yyleng,
			type
		});
	}
}

/*
 * Fetch the current lexer state and throw an error.
 */
TokenizeError NyanLexer::error(const std::string &msg) {
	return TokenizeError{
		NyanLocation{
			this->file,
			this->yylineno,
			this->linepos - this->yyleng
		},
		msg
	};
}


void NyanLexer::advance_linepos() {
	this->linepos += this->yyleng;
}


void NyanLexer::reset_linepos() {
	this->linepos = this->linepos_start;
}

/**
 * measure the indentation of a line
 */
int NyanLexer::handle_indent(const char *line) {

	// measure indent
	int depth = 0;
	while (*line == ' ') {
		depth++;
		line++;
	}

	if ((depth % SPACES_PER_INDENT) > 0) {
		throw this->error("indentation requires exactly "
		                  SPACES_PER_INDENT_STR
		                  " spaces per level");
	}

	// Indentation depth of the last line
	int last_depth = 0;

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
					this->token(token_type::DEDENT);
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
				this->token(token_type::INDENT);
			}
		}
	}

	return depth - last_depth;
}


LexerError::LexerError(const NyanLocation &location,
                       const std::string &msg)
	:
	NyanFileError{location, msg} {}

} // namespace nyan
