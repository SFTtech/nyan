// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_state.h"

#include <algorithm>
#include <sstream>

#include "change_tracker.h"
#include "compiler.h"
#include "object_info.h"
#include "util.h"


namespace nyan {


ObjectState::ObjectState(std::deque<fqon_t> &&parents)
	:
	parents{std::move(parents)} {}


void ObjectState::apply(const std::shared_ptr<ObjectState> &mod,
                        const ObjectInfo &mod_info,
                        ObjectChanges &tracker) {

	const auto &inher_changes = mod_info.get_inheritance_change();
	if (inher_changes.size() > 0) {
		for (auto &change : inher_changes) {

			bool parent_exists = (
				std::find(
					std::begin(this->parents),
					std::end(this->parents),
					change.get_target()
				) != std::end(this->parents)
			);

			// only add the parent if it does not exist.
			// maybe we may want to relocate it in the future?
			if (not parent_exists) {
				switch (change.get_type()) {
				case inher_change_t::ADD_FRONT:
					this->parents.push_front(change.get_target());
					tracker.add_parent(change.get_target());
					break;
				case inher_change_t::ADD_BACK:
					this->parents.push_back(change.get_target());
					tracker.add_parent(change.get_target());
					break;
				default:
					throw InternalError{"unsupported inheritance change type"};
				}
			}
		}
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

		// TODO optimization: we could now calculate the resulting value!
		// TODO: invalidate value cache with the change tracker
	}
}


std::shared_ptr<ObjectState> ObjectState::copy() const {
	return std::make_shared<ObjectState>(*this);
}


const std::deque<fqon_t> &ObjectState::get_parents() const {
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


std::string ObjectState::str() const {
	std::ostringstream builder;

	builder << "ObjectState("
	        << util::strjoin(", ", this->parents)
	        << ")"
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
