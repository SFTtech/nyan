// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_FILE_H_
#define NYAN_NYAN_FILE_H_

#include <string>


namespace nyan {


/**
 * Represents a nyan data file.
 */
class NyanFile {
public:
	NyanFile(const std::string &path);
	NyanFile(const std::string &virtual_name, const std::string &data);
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
};

} // namespace std

#endif
