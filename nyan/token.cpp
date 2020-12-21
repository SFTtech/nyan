// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "token.h"

#include <sstream>

#include "error.h"
#include "file.h"


namespace nyan {


Token::Token(const std::shared_ptr<File> &file,
             int line, int line_offset, int length,
             token_type type)
	:
	location{file, line, line_offset, length},
	type{type} {}


Token::Token(const std::shared_ptr<File> &file,
             int line, int line_offset, int length,
             token_type type, const std::string &value)
	:
	location{file, line, line_offset, length},
	type{type},
	value{value} {}


Token::Token()
	:
	type{token_type::INVALID} {}


const std::string &Token::get() const {
	return this->value;
}


std::string Token::str() const {
	std::ostringstream builder;
	builder << "(" << this->location.get_line() << ":"
	        << this->location.get_line_offset() << ": "
	        << token_type_str(this->type);
	if (this->value.size() > 0) {
		builder << " '" << this->value << "'";
	}
	builder << ")";
	return builder.str();
}


bool Token::exists() const {
	return this->get().size() > 0;
}


bool Token::is_endmarker() const {
	switch (this->type) {
	case token_type::ENDFILE:
	case token_type::ENDLINE:
		return true;

	case token_type::INVALID:
		throw InternalError{"invalid token used"};

	default:
		return false;
	}
}


bool Token::is_content() const {
	switch (this->type) {
	case token_type::FLOAT:
	case token_type::ID:
	case token_type::INF:
	case token_type::INT:
	case token_type::STRING:
		return true;

	case token_type::INVALID:
		throw InternalError{"invalid token used"};

	default:
		return false;
	}
}

} // namespace nyan
