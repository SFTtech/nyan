// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_token.h"

#include <sstream>

#include "nyan_file.h"


namespace nyan {


NyanToken::NyanToken(const NyanFile &file,
                     int line, int line_offset,
                     token_type type)
	:
	location{file, line, line_offset},
	type{type} {}


NyanToken::NyanToken(const NyanFile &file,
                     int line, int line_offset,
                     token_type type, const std::string &value)
	:
	location{file, line, line_offset},
	type{type},
	value{value} {}


NyanToken::NyanToken()
	:
	type{token_type::INVALID} {}


const std::string &NyanToken::get() const {
	return this->value;
}

std::string NyanToken::str() const {
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


TokenizeError::TokenizeError(const NyanLocation &location,
                             const std::string &msg)
	:
	NyanFileError{location, msg} {}

} // namespace nyan
