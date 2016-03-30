// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_util.h"

#include <cstring>
#include <errno.h>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>


namespace nyan {
namespace util {

std::string read_file(const std::string &filename) {
	std::ifstream input(filename);

	if (input) {
		std::string ret;

		input.seekg(0, std::ios::end);
		ret.resize(input.tellg());
		input.seekg(0, std::ios::beg);
		input.read(&ret[0], ret.size());
		input.close();

		return ret;
	} else {
		std::ostringstream builder;
		builder << "failed reading file '"
		        << filename << "': "
		        << strerror(errno);
		throw NyanError{builder.str()};
	}
}

} // namespace util
} // namespace nyan
