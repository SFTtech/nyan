// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_TOKEN_H_
#define NYAN_NYAN_TOKEN_H_

#include <string>

#include "nyan_error.h"

namespace nyan {

/**
 * Available tokens
 */
enum class token_type {
	COLON,
	COMMA,
	DEDENT,
	ENDFILE,
	ENDLINE,
	FLOAT,
	ID,
	ILLEGAL,
	INDENT,
	INT,
	LANGLE,
	LBRACE,
	LBRACKET,
	LPAREN,
	OPERATOR,
	PASS,
	RANGLE,
	RBRACE,
	RBRACKET,
	RPAREN,
	STRING,
};

/**
 * Text representations of the token types.
 * Oh you, c++, such a good and comfort language.
 */
constexpr const char *token_type_str(token_type type) {
	using namespace std::string_literals;

	switch (type) {
	case token_type::COLON:          return "colon";
	case token_type::COMMA:          return "comma";
	case token_type::DEDENT:         return "dedentation";
	case token_type::ENDFILE:        return "end of file";
	case token_type::ENDLINE:        return "end of line";
	case token_type::FLOAT:          return "float";
	case token_type::ID:             return "identifier";
	case token_type::ILLEGAL:        return "illegal";
	case token_type::INDENT:         return "indentation";
	case token_type::INT:            return "int";
	case token_type::LANGLE:         return "<";
	case token_type::LBRACE:         return "{";
	case token_type::LBRACKET:       return "[";
	case token_type::LPAREN:         return "(";
	case token_type::OPERATOR:       return "operator";
	case token_type::PASS:           return "pass";
	case token_type::RANGLE:         return ">";
	case token_type::RBRACE:         return "}";
	case token_type::RBRACKET:       return "]";
	case token_type::RPAREN:         return ")";
	case token_type::STRING:         return "string";
	}
}


/**
 * These are spit out by the nyan lexer.
 */
class NyanToken {
public:
	NyanToken(int line,
	          int line_offset,
	          token_type type,
	          const std::string &value="");
	virtual ~NyanToken() = default;

	std::string str() const;

	int line;
	int line_offset;
	token_type type;
	std::string value;
};


/**
 * Tokenize failure
 */
class TokenizeError : public ParserError {
public:
	TokenizeError(const std::string &msg, int line, int line_offset);
	virtual ~TokenizeError() = default;
};

} // namespace nyan

#endif
