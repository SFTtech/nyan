// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_state.h"

#include <sstream>

#include "util.h"


namespace nyan {

// TODO: asdf copy-constructor to copy all members
//       to the new object state. Member already has the right constructors.

ObjectState::ObjectState(std::vector<fqon_t> &&parents)
	:
	parents{std::move(parents)} {}


void ObjectState::apply(const std::shared_ptr<ObjectState> &other) {
	// change each member in this object by the member of the patch.
	// other->members: maps memberid_t name => Member
	for (auto &it : other->members) {
		auto search = this->members.find(it.first);
		if (search == std::end(this->members)) {
			throw Error{"tried to patch an unknown member"};
		}
		search->second.apply(it.second);
	}
}


std::shared_ptr<ObjectState> ObjectState::copy() const {
	return std::make_shared<ObjectState>(*this);
}


const std::vector<fqon_t> &ObjectState::get_parents() const {
	return this->parents;
}


bool ObjectState::has_member(const memberid_t &name) const {
	return this->members.find(name) != std::end(this->members);
}


const Member *ObjectState::get_member(const memberid_t &name) const {
	auto it = this->members.find(name);
	if (it == std::end(this->members)) {
		return nullptr;
	}
	return &it->second;
}


const std::vector<fqon_t> &ObjectState::get_linearization() const {
	return this->linearization;
}


void ObjectState::set_linearization(std::vector<fqon_t> &&lin) {
	this->linearization = std::move(lin);
}


std::string ObjectState::str() const {
	std::ostringstream builder;

	builder << "ObjectState("
	        << util::strjoin(", ", this->parents)
	        << ")["
	        << util::strjoin(", ", this->linearization)
	        << "]:"
	        << std::endl;

	if (this->members.size() == 0) {
		builder << "    [no members]" << std::endl;
	}

	for (auto &it : this->members) {
		builder << "    " << it.first
		        << " -> " << it.second.str() << std::endl;
	}

	return builder.str();
}


void ObjectState::set_members(std::unordered_map<memberid_t, Member> &&members) {
	this->members = std::move(members);
}

} // namespace nyan
