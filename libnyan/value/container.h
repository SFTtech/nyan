// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_CONTAINER_H_
#define NYAN_NYAN_VALUE_CONTAINER_H_

#include "../ptr_container.h"
#include "value.h"


namespace nyan {


/**
 * Container class to support owning and non-owning values.
 */
class ValueContainer : public PtrContainer<Value> {
	using PtrContainer<Value>::PtrContainer;
};


} // namespace nyan


namespace std {

/**
 * Hash for ValueContainers.
 */
template<>
struct hash<nyan::ValueContainer> {
	size_t operator ()(const nyan::ValueContainer &val) const;
};

} // namespace std

#endif
