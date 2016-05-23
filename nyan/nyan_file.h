// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_FILE_H_
#define NYAN_NYAN_FILE_H_

#include <string>
#include <vector>


namespace nyan {


/**
 * Represents a nyan data file.
 */
class NyanFile {
	/**
	 * Line information struct.
	 * Stores the offset in the file and the line length.
	 */
	struct line_info {
		size_t offset;
		size_t len;
	};

public:
	NyanFile(const std::string &path);
	NyanFile(const std::string &virtual_name, std::string &&data);

	// moving allowed
	NyanFile(NyanFile &&other);
	NyanFile& operator =(NyanFile &&other);

	// no copies
	NyanFile(const NyanFile &other) = delete;
	NyanFile &operator =(const NyanFile &other) = delete;

	virtual ~NyanFile() = default;

	/**
	 * Return the file name.
	 */
	const std::string &get_name() const;

	/**
	 * Return the file content.
	 */
	const std::string &get() const;

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
	std::string name;
	std::string data;

	std::vector<line_info> lines;
};

} // namespace std

#endif
