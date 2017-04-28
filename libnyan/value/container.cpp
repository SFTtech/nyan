// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "container.h"

namespace std {

size_t hash<nyan::ValueContainer>::operator ()(
	const nyan::ValueContainer &val) const {

	return val->hash();
}

} // namespace std
