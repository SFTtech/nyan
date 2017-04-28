// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "location.h"

#include "file.h"
#include "token.h"

namespace nyan {


Location::Location(const Token &token)
	:
	Location{token.location} {}


Location::Location(const File &file,
                           int line, int line_offset)
	:
	file{&file},
	line{line},
	line_offset{line_offset} {}


int Location::get_line() const {
	return this->line;
}


int Location::get_line_offset() const {
	return this->line_offset;
}


std::string Location::get_line_content() const {
	return this->file->get_line(this->get_line());
}


void Location::str(std::ostringstream &builder) const {
	builder << this->file->get_name() << ":"
	        << this->line << ":"
	        << this->line_offset << ": ";
}

} // namespace nyan
