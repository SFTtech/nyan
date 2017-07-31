// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_state.h"

#include <algorithm>
#include <sstream>

#include "object_info.h"
#include "util.h"


namespace nyan {

// TODO: asdf copy-constructor to copy all members
//       to the new object state. Member already has the right constructors.

ObjectState::ObjectState(std::vector<fqon_t> &&parents)
	:
	parents{std::move(parents)} {}


bool ObjectState::apply(const std::shared_ptr<ObjectState> &mod,
                        const ObjectInfo &mod_info) {

	bool parents_changed = false;

	const std::vector<fqon_t> &newparents = mod_info.get_inheritance_add();
	if (newparents.size() > 0) {
		for (auto &newparent : newparents) {
			auto it = std::find(std::begin(this->parents),
			                    std::end(this->parents),
			                    newparent);

			// don't add the parent if it's already there.
			if (it == std::end(this->parents)) {
				this->parents.push_back(newparent);
			}
		}

		// recalculate inheritance hierarchy.
		parents_changed = true;
	}

	// change each member in this object by the member of the patch.
	// other->members: maps memberid_t name => Member
	for (auto &it : mod->members) {
		auto search = this->members.find(it.first);
		if (search == std::end(this->members)) {
			// copy the member from the modification object,
			// if it is a patch.
			// that way a child object without the member
			// can get the modifications.
			if (likely(mod_info.is_patch())) {
				this->members.emplace(it.first, it.second);
			}
			else {
				throw InternalError{
					"a non-patch tried to change a nonexisting member"
				};
			}
		}
		else {
			search->second.apply(it.second);
		}

		// TODO: we could now calculate the resulting value!
	}

	return parents_changed;
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


Member *ObjectState::get_member(const memberid_t &name) {
	auto it = this->members.find(name);
	if (it == std::end(this->members)) {
		return nullptr;
	}
	return &it->second;
}


// Thanks C++, always redundancy free!
const Member *ObjectState::get_member(const memberid_t &name) const {
	auto it = this->members.find(name);
	if (it == std::end(this->members)) {
		return nullptr;
	}
	return &it->second;
}


const std::unordered_map<memberid_t, Member> &ObjectState::get_members() const {
	return this->members;
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
