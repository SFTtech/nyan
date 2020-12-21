// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "inheritance_change.h"

namespace nyan {

InheritanceChange::InheritanceChange(inher_change_t type, fqon_t &&target)
	:
	type{type},
	target{std::move(target)} {}


inher_change_t InheritanceChange::get_type() const {
	return this->type;
}


const fqon_t &InheritanceChange::get_target() const {
	return this->target;
}

} // namespace nyan
