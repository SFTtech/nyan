// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <vector>

#include "config.h"

namespace nyan {

/**
 * Information about a patch.
 */
class PatchInfo {
public:
	explicit PatchInfo(fqon_t &&target);
	~PatchInfo() = default;

	const fqon_t &get_target() const;

	std::string str() const;

protected:
	/**
	 * Patch target name.
	 */
	fqon_t target;
};


} // namespace nyan
