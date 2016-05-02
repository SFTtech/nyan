// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_location.h"

#include "nyan_file.h"
#include "nyan_token.h"

namespace nyan {


NyanLocation::NyanLocation(const NyanToken &token)
	:
	NyanLocation{token.location} {}


NyanLocation::NyanLocation(const NyanFile &file,
                           int line, int line_offset)
	:
	filename{file.get_name()},
	line{line},
	line_offset{line_offset} {}


int NyanLocation::get_line() const {
	return this->line;
}


int NyanLocation::get_line_offset() const {
	return this->line_offset;
}


void NyanLocation::str(std::ostringstream &builder) const {
	builder << this->filename << ":"
	        << this->line << ":"
	        << this->line_offset << ": ";
}

} // namespace nyan
