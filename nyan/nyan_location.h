// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_LOCATION_H_
#define NYAN_NYAN_LOCATION_H_

#include <sstream>
#include <string>

namespace nyan {

class NyanFile;
class NyanToken;

/**
 * Location of some data in nyan.
 * Used to display error messages for positions in the file.
 */
class NyanLocation {
public:
	NyanLocation() = default;
	NyanLocation(const NyanToken &token);
	NyanLocation(const NyanFile &file, int line, int line_offset);
	virtual ~NyanLocation() = default;

	int get_line() const;
	int get_line_offset() const;

	std::string get_line_content() const;

	void str(std::ostringstream &builder) const;

protected:
	const NyanFile *file;
	int line;
	int line_offset;
};

} // namespace nyan

#endif
