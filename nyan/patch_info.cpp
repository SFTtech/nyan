// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "patch_info.h"

#include <sstream>

#include "error.h"
#include "util.h"


namespace nyan {

PatchInfo::PatchInfo(fqon_t &&target)
	:
	target{std::move(target)} {}


const fqon_t &PatchInfo::get_target() const {
	return this->target;
}


std::string PatchInfo::str() const {
	std::ostringstream builder;

	builder << "<" << this->target << ">";
	return builder.str();
}

} // namespace nyan
