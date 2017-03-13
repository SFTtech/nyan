// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_CONTAINER_H_
#define NYAN_NYAN_VALUE_CONTAINER_H_

#include "nyan_ptr_container.h"
#include "nyan_value.h"


namespace nyan {


/**
 * Container class to support owning and non-owning values.
 */
class NyanValueContainer : public NyanPtrContainer<NyanValue> {
	using NyanPtrContainer<NyanValue>::NyanPtrContainer;
};


} // namespace nyan


namespace std {

/**
 * Hash for NyanValueContainers.
 */
template<>
struct hash<nyan::NyanValueContainer> {
	size_t operator ()(const nyan::NyanValueContainer &val) const;
};

} // namespace std

#endif
