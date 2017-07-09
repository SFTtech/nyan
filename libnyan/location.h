// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <sstream>
#include <string>

namespace nyan {

class File;
class Token;
class IDToken;


/**
 * Location of some data in nyan.
 * Used to display error messages for positions in the file.
 */
class Location {
public:
	Location() = default;
	Location(const Token &token);
	Location(const IDToken &token);
	Location(const std::shared_ptr<File> &file, int line,
	         int line_offset, int length=0);
	explicit Location(const std::string &custom);

	~Location() = default;

	int get_line() const;
	int get_line_offset() const;
	int get_length() const;

	std::string get_line_content() const;

	void str(std::ostringstream &builder) const;

protected:
	std::shared_ptr<File> file;

	int line;
	int line_offset;
	int length;

	std::string msg;
};

} // namespace nyan
