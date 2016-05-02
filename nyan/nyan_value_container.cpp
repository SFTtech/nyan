// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_container.h"

namespace std {

size_t hash<nyan::NyanValueContainer>::operator ()(
	const nyan::NyanValueContainer &val) const {

	return val->hash();
}

} // namespace std
