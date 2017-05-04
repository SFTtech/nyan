// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <sstream>
#include <string>

namespace nyan {

class File;
class Token;

/**
 * Location of some data in nyan.
 * Used to display error messages for positions in the file.
 */
class Location {
public:
	Location() = default;
	Location(const Token &token);
	Location(const File &file, int line, int line_offset);
	~Location() = default;

	int get_line() const;
	int get_line_offset() const;

	std::string get_line_content() const;

	void str(std::ostringstream &builder) const;

protected:
	const File *file;
	int line;
	int line_offset;
	// TODO: range-locations
};

} // namespace nyan
