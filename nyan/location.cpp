// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "location.h"

#include "file.h"
#include "token.h"
#include "id_token.h"


namespace nyan {


Location::Location(const Token &token)
	:
	Location{token.location} {}


Location::Location(const IDToken &token)
	:
	Location{token.get_start_location()} {

	// use the full id length as location length
	this->length = token.get_length();
}


Location::Location(const std::shared_ptr<File> &file,
                   int line, int line_offset, int length)
	:
	file{file},
	line{line},
	line_offset{line_offset},
	length{length} {}


Location::Location(const std::string &custom)
	:
	_is_builtin{true},
	msg{custom} {}


bool Location::is_builtin() const {
	return this->_is_builtin;
}

const std::string &Location::get_msg() const {
	return this->msg;
}

int Location::get_line() const {
	return this->line;
}


int Location::get_line_offset() const {
	return this->line_offset;
}

int Location::get_length() const {
	return this->length;
}


std::string Location::get_line_content() const {
	if (this->_is_builtin) {
		return this->msg;
	}
	return this->file->get_line(this->get_line());
}


void Location::str(std::ostringstream &builder) const {
	if (this->_is_builtin) {
		builder << "[native call]: ";
		return;
	}

	builder << this->file->get_name() << ":"
	        << this->line << ":"
	        << this->line_offset << ": ";
}

} // namespace nyan
