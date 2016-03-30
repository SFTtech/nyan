// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_token.h"

#include <sstream>

namespace nyan {

TokenizeError::TokenizeError(const std::string &msg,
                             int line, int line_offset)
	:
	ParserError{msg, line, line_offset} {}

NyanToken::NyanToken(int line, int line_offset,
                     token_type type, const std::string &value)
	:
	line{line},
	line_offset{line_offset},
	type{type},
	value{value} {}


std::string NyanToken::str() const {
	std::ostringstream builder;
	builder << "(" << this->line << ":" << this->line_offset << ": "
	        << token_type_str(this->type) << " '" << this->value << "')";
	return builder.str();
}


} // namespace nyan
