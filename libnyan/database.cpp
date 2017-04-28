// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "database.h"


#include "object.h"


using namespace std::string_literals;


namespace nyan {

Database::Database(Database *parent)
	:
	parent{parent} {}

Database::~Database() {}


Object *Database::add(std::unique_ptr<Object> &&obj) {
	const std::string &name = obj->get_name();

	auto it = this->objects.find(name);
	if (it == std::end(this->objects)) {
		auto ins = this->objects.insert(std::make_pair(name, std::move(obj)));
		if (std::get<1>(ins) != true) {
			throw InternalError{"couldn't add object to namespace"};
		}

		// we get the iterator back, now return the pointer where it
		// reported the insertion of the nyanobject.
		return ((*std::get<0>(ins)).second).get();
	}
	else {
		throw Error{"Object already in store: '"s + name + "'"};
	}
}



Object *Database::get(const std::string &name) const {
	return nullptr;
}


} // namespace nyan
