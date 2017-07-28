// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "meta_info.h"

#include <sstream>

#include "error.h"


namespace nyan {

ObjectInfo &MetaInfo::add_object(const fqon_t &name, ObjectInfo &&obj) {
	auto ret = this->object_info.insert({name, std::move(obj)});
	if (ret.second == false) {
		throw Error{name + ": object already in metainfo store"};
	}

	return ret.first->second;
}


const MetaInfo::obj_info_t &MetaInfo::get_objects() const {
	return this->object_info;
}


ObjectInfo *MetaInfo::get_object(const fqon_t &name) {
	auto it = this->object_info.find(name);
	if (it == std::end(this->object_info)) {
		return nullptr;
	}
	return &it->second;
}


// Thanks C++ for the beautiful duplication
const ObjectInfo *MetaInfo::get_object(const fqon_t &name) const {
	auto it = this->object_info.find(name);
	if (it == std::end(this->object_info)) {
		return nullptr;
	}
	return &it->second;
}


bool MetaInfo::has_object(const fqon_t &name) const {
	return (this->object_info.find(name) != std::end(this->object_info));
}


std::string MetaInfo::str() const {
	std::ostringstream builder;

	for (auto &it : this->get_objects()) {
		builder << it.first << " -> " << it.second.str() << std::endl;
	}

	return builder.str();
}

} // namespace nyan
