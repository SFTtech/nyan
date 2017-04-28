// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_TYPE_CONTAINER_H_
#define NYAN_NYAN_TYPE_CONTAINER_H_

#include "ptr_container.h"
#include "type.h"


namespace nyan {


/**
 * Container class to contain a Type, either by
 * owning one or by referencing an existing one.
 */
class TypeContainer : public PtrContainer<Type> {
	using PtrContainer<Type>::PtrContainer;
};


} // namespace nyan

#endif
