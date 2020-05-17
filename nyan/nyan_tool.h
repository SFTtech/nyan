// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>
#include <unordered_map>

namespace nyan {

/**
 * boolean flags to be set by cmdline options
 */
enum class option_flag {
	ECHO,
	TEST_PARSER
};

/**
 * string arguments to be set by cmdline options
 */
enum class option_param {
	FILE
};

using flags_t = std::unordered_map<option_flag, bool>;
using params_t = std::unordered_map<option_param, std::string>;


/**
 * Run the nyan tool.
 */
int run(flags_t, params_t);

/**
 * Display the tool help.
 */
void help();


} // namespace nyan

namespace std {

/**
 * Hash for the option_flag enum class. Fak u C++!
 */
template<>
struct hash<nyan::option_flag> {
	size_t operator ()(const nyan::option_flag &x) const {
		return static_cast<size_t>(x);
	}
};

/**
 * Hash for the option_param enum class. Fak u C++!
 */
template<>
struct hash<nyan::option_param> {
	size_t operator ()(const nyan::option_param &x) const {
		return static_cast<size_t>(x);
	}
};

} // namespace std
