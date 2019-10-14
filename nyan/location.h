// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
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

	bool is_builtin() const;
	const std::string &get_msg() const;
	int get_line() const;
	int get_line_offset() const;
	int get_length() const;

	std::string get_line_content() const;

	void str(std::ostringstream &builder) const;

protected:
	/**
	 * if true, this location does point to a file,
	 * instead it describes some built-in location of nyan itself.
	 */
	bool _is_builtin = false;

	std::shared_ptr<File> file;

	int line;
	int line_offset;
	int length;

	std::string msg;
};

} // namespace nyan
