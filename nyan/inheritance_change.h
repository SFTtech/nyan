// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "config.h"
#include "ops.h"


namespace nyan {

/**
 * Runtime inheritance change.
 */
class InheritanceChange {
public:
	InheritanceChange(inher_change_t type, fqon_t &&target);

	inher_change_t get_type() const;
	const fqon_t &get_target() const;

protected:
	inher_change_t type;
	fqon_t target;
};


} // namespace nyan
