// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>

#include "error.h"
#include "location.h"

namespace nyan {

class File;

/**
 * Available tokens
 */
enum class token_type {
	AS,
	COLON,
	COMMA,
	DEDENT,
	ENDFILE,
	ENDLINE,
	ELLIPSIS,
	FLOAT,
	FROM,
	ID,
	IMPORT,
	INDENT,
	INVALID,
	INT,
	LANGLE,
	LBRACE,
	LBRACKET,
	LPAREN,
	MISSING,
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
	case token_type::AS:             return "as";
	case token_type::COLON:          return "colon";
	case token_type::COMMA:          return "comma";
	case token_type::DEDENT:         return "dedentation";
	case token_type::ELLIPSIS:       return "ellipsis";
	case token_type::ENDFILE:        return "end of file";
	case token_type::ENDLINE:        return "end of line";
	case token_type::FLOAT:          return "float";
	case token_type::FROM:           return "from";
	case token_type::ID:             return "identifier";
	case token_type::IMPORT:         return "import";
	case token_type::INDENT:         return "indentation";
	case token_type::INT:            return "int";
	case token_type::INVALID:        return "invalid";
	case token_type::LANGLE:         return "<";
	case token_type::LBRACE:         return "{";
	case token_type::LBRACKET:       return "[";
	case token_type::LPAREN:         return "(";
	case token_type::MISSING:        return "missing";
	case token_type::OPERATOR:       return "operator";
	case token_type::PASS:           return "pass";
	case token_type::RANGLE:         return ">";
	case token_type::RBRACE:         return "}";
	case token_type::RBRACKET:       return "]";
	case token_type::RPAREN:         return ")";
	case token_type::STRING:         return "string";
	}

	return "unhandled token_type";
}


/**
 * Return if the given token type requires a payload storage.
 * If not, then the token type is already enough information.
 */
constexpr bool token_needs_payload(token_type type) {

	switch (type) {
	case token_type::FLOAT:
	case token_type::ID:
	case token_type::INT:
	case token_type::OPERATOR:
	case token_type::STRING:
		return true;
	default:
		return false;
	}
}


/**
 * These are spit out by the nyan lexer.
 */
class Token {
public:
	Token();
	Token(const File &file,
	          int line,
	          int line_offset,
	          token_type type);
	Token(const File &file,
	          int line,
	          int line_offset,
	          token_type type,
	          const std::string &value);
	~Token() = default;

	std::string str() const;
	bool exists() const;

	const std::string &get() const;

	Location location;
	token_type type;

protected:
	std::string value;
};


/**
 * Tokenize failure
 */
class TokenizeError : public FileError {
public:
	TokenizeError(const Location &location,
	              const std::string &msg);
	virtual ~TokenizeError() = default;
};

} // namespace nyan
