// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "util.h"

#include <cstring>
#ifndef _MSC_VER
#include <cxxabi.h>
#include <dlfcn.h>
#endif
#include <errno.h>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>


namespace nyan {
namespace util {

size_t file_size(const std::string &filename) {
	std::ifstream input{filename, std::ifstream::ate | std::ifstream::binary};

	if (input) {
		size_t ret = input.tellg();
		input.close();
		return ret;
	}
	else {
		std::ostringstream builder;
		builder << "failed getting size of file '"
		        << filename << "': "
		        << strerror(errno);
		throw FileReadError{builder.str()};
	}
}


std::string read_file(const std::string &filename, bool binary) {
	std::ifstream::openmode mode = std::ifstream::in | std::ifstream::ate;
	if (binary) {
		mode |= std::ifstream::binary;
	}

	std::ifstream input{filename, mode};

	if (input) {
		std::string ret;

		// we're at the file end already because of ifstream::ate
		ret.resize(input.tellg());
		input.seekg(0, std::ios::beg);
		input.read(&ret[0], ret.size());
		input.close();

		return ret;
	}
	else {
		std::ostringstream builder;
		builder << "failed reading file '"
		        << filename << "': "
		        << strerror(errno);
		throw FileReadError{builder.str()};
	}
}


std::string demangle(const char *symbol) {
#ifdef _MSC_VER
	// TODO: demangle names for MSVC; Possibly using UnDecorateSymbolName
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms681400(v=vs.85).aspx
	return symbol;
#else
	int status;
	char *buf = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);

	if (status != 0) {
		return symbol;
	} else {
		std::string result{buf};
		free(buf);
		return result;
	}
#endif
}


std::string addr_to_string(const void *addr) {
	std::ostringstream out;
	out << "[" << addr << "]";
	return out.str();
}


std::string symbol_name(const void *addr,
                        bool require_exact_addr, bool no_pure_addrs) {
#ifdef _MSC_VER
	// TODO: implement symbol_name for MSVC; Possibly using SymFromAddr
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms681323(v=vs.85).aspx
	return no_pure_addrs ? "" : addr_to_string(addr);
#else
	Dl_info addr_info;

	if (dladdr(addr, &addr_info) == 0) {
		// dladdr has... failed.
		return no_pure_addrs ? "" : addr_to_string(addr);
	} else {
		size_t symbol_offset = reinterpret_cast<size_t>(addr) -
		                       reinterpret_cast<size_t>(addr_info.dli_saddr);

		if (addr_info.dli_sname == nullptr or
		    (symbol_offset != 0 and require_exact_addr)) {

			return no_pure_addrs ? "" : addr_to_string(addr);
		}

		if (symbol_offset == 0) {
			// this is our symbol name.
			return demangle(addr_info.dli_sname);
		} else {
			std::ostringstream out;
			out << demangle(addr_info.dli_sname)
			    << "+0x" << std::hex
			    << symbol_offset << std::dec;
			return out.str();
		}
	}
#endif
}


std::vector<std::string> split(const std::string &txt, char delimiter) {
	std::vector<std::string> items;
	// use the back inserter iterator and the templated split function.
	split(txt, delimiter, std::back_inserter(items));
	return items;
}


bool ends_with(const std::string &txt, const std::string &end) {
	if (end.size() > txt.size()) {
		return false;
	}
	return std::equal(end.rbegin(), end.rend(), txt.rbegin());
}


size_t hash_combine(size_t hash1, size_t hash2) {
	// Taken from http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
	return hash1 ^ (hash2 + 0x9e3779b9 + ((hash1 << 6) + (hash1 >> 2)));
}

} // namespace util
} // namespace nyan
