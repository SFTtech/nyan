// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_database.h"


#include "nyan_object.h"


using namespace std::string_literals;


namespace nyan {

NyanDatabase::NyanDatabase(NyanDatabase *parent)
	:
	parent{parent} {}

NyanDatabase::~NyanDatabase() {}


NyanObject *NyanDatabase::add(std::unique_ptr<NyanObject> &&obj) {
	const std::string &name = obj->get_name();

	auto it = this->objects.find(name);
	if (it == std::end(this->objects)) {
		auto ins = this->objects.insert(std::make_pair(name, std::move(obj)));
		if (std::get<1>(ins) != true) {
			throw NyanInternalError{"couldn't add object to namespace"};
		}

		// we get the iterator back, now return the pointer where it
		// reported the insertion of the nyanobject.
		return ((*std::get<0>(ins)).second).get();
	}
	else {
		throw NyanError{"NyanObject already in store: '"s + name + "'"};
	}
}



NyanObject *NyanDatabase::get(const std::string &name) const {
	return nullptr;
}


} // namespace nyan
