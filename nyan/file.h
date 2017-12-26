// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>
#include <vector>


namespace nyan {


/**
 * Represents a nyan data file.
 */
class File {
public:
	File(const std::string &path);
	File(const std::string &virtual_name, std::string &&data);

	// moving allowed
	File(File &&other) noexcept = default;
	File& operator =(File &&other) noexcept = default;

	// no copies
	File(const File &other) = delete;
	File &operator =(const File &other) = delete;

	virtual ~File() = default;

	/**
	 * Return the file name.
	 */
	const std::string &get_name() const;

	/**
	 * Return the file content.
	 */
	const std::string &get_content() const;

	/**
	 * Return the given line number of the file.
	 * Starts at line 1. *buhuuuuu* *sob* *mrrrmmuu* *whimper*
	 */
	std::string get_line(size_t n) const;

	/**
	 * Return the number of lines in the file.
	 * It will always have at least one line.
	 * The empty file has one line of length 0.
	 */
	size_t get_line_count() const;

	/**
	 * Return a c string of the file content.
	 */
	const char *c_str() const;

	/**
	 * Return the size of the file content.
	 */
	size_t size() const;

protected:
	/**
	 * Create line_ends entries from the file content.
	 */
	void extract_lines();

	std::string name;
	std::string data;

	/**
	 * Stores the offsets of line endings in the file content.
	 */
	std::vector<size_t> line_ends;
};

} // namespace std
