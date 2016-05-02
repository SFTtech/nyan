// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_store.h"


namespace nyan {

NyanStore::NyanStore() {}


NyanObject *NyanStore::add(std::unique_ptr<NyanObject> &&obj) {
	NyanNamespace *space = &this->root;
	return space->add_obj(std::move(obj));
}


NyanObject *NyanStore::get(const std::string &name) const {
	// TODO: namespace resolution
	const NyanNamespace *space = &this->root;
	return space->get_obj(name);
}


} // namespace nyan
