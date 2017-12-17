// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "impl.h"


namespace nyan::lexer {

Impl::Impl(Lexer *outer)
	: // set the input stream in the flex base class
	yyFlexLexer{&outer->input, nullptr},
	owner{outer} {
}

/*
 * Fetch the current lexer state and throw an error.
 */
TokenizeError Impl::error(const std::string &msg) {
	return TokenizeError{
		Location{
			this->owner->file,
			this->yylineno,
			this->linepos - this->yyleng,
			this->yyleng
		},
		msg
	};
}

void Impl::reset_linepos() {
	this->linepos = linepos_start;
}

void Impl::endline() {
	this->yylineno--;
	this->token(token_type::ENDLINE);
	this->yylineno++;
	this->reset_linepos();
}


/*
 * Fetch the current lexer state variables and create a token.
 */
void Impl::token(token_type type) {

	int token_start = this->linepos - this->yyleng;

	// to register open and close parenthesis
	// for correct line-wrap-indentation.
	this->owner->track_brackets(type, token_start);

	if (token_needs_payload(type)) {
		this->owner->tokens.push(Token{
			this->owner->file,
			this->yylineno,
			token_start,
			this->yyleng,
			type,
			this->yytext
		});
	}
	else {
		this->owner->tokens.push(Token{
			this->owner->file,
			this->yylineno,
			token_start,
			this->yyleng,
			type
		});
	}
}

} // namespace nyan::lexer
