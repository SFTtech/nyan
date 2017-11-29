// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>

namespace nyan {

/** number of spaces per indent **/
constexpr int SPACES_PER_INDENT = 4;

/** ordering type */
using order_t = uint64_t;

/** starting point of order */
constexpr const order_t DEFAULT_T = 0;

/** fully-qualified object name */
using fqon_t = std::string;

/** member name identifier type */
using memberid_t = std::string;

/** member and override nesting depth type */
using override_depth_t = unsigned;


} // nyan
