// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "namespace.h"

#include "error.h"

namespace nyan {

NyanNamespace::NyanNamespace() {}


NyanNamespace *NyanNamespace::add_namespace(const std::string &name) {
	throw NyanInternalError{"TODO add_namespace"};
}


NyanObject *NyanNamespace::get_obj(const std::string &name) const {
	return nullptr;
}


NyanNamespace *NyanNamespace::get_namespace(const std::string &name) const {
	return nullptr;
}

} // namespace nyan
