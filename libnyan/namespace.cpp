// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "namespace.h"

#include "error.h"

namespace nyan {

Namespace::Namespace() {}


Namespace *Namespace::add_namespace(const std::string &name) {
	throw InternalError{"TODO add_namespace"};
}


Object *Namespace::get_obj(const std::string &name) const {
	return nullptr;
}


Namespace *Namespace::get_namespace(const std::string &name) const {
	return nullptr;
}

} // namespace nyan
