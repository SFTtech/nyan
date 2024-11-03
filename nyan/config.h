// Copyright 2017-2023 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once

#ifdef _MSC_VER
	// Allow using alternative operator representation with non-conforming compiler
	#include <ciso646>
#endif

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

namespace nyan {

/** number of spaces per indent **/
constexpr int SPACES_PER_INDENT = 4;

/** ordering type */
using order_t = uint64_t;

/** starting point of order */
constexpr const order_t DEFAULT_T = 0;

/** the maximum representable value of order_t is always the "latest" value */
constexpr const order_t LATEST_T = std::numeric_limits<order_t>::max();

/** fully-qualified object name */
using fqon_t = std::string;

/** fully-qualified namespace name */
using fqnn_t = fqon_t;

/** member name identifier type */
using memberid_t = std::string;

/** member and override nesting depth type */
using override_depth_t = unsigned;

/** type used for nyan::Int values */
using value_int_t = int64_t;

/** type used for nyan::Float values */
using value_float_t = double;

} // namespace nyan
