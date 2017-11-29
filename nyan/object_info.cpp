// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_info.h"

#include <sstream>

#include "error.h"
#include "util.h"
#include "patch_info.h"
#include "state.h"


namespace nyan {

ObjectInfo::ObjectInfo(const Location &location)
	:
	location{location},
	initial_patch{false} {}


const Location &ObjectInfo::get_location() const {
	return this->location;
}


MemberInfo &ObjectInfo::add_member(const memberid_t &name,
                                   MemberInfo &&member) {

	// copy the location so it's still valid if the insert fails.
	Location loc = member.get_location();

	auto ret = this->member_info.insert({name, std::move(member)});
	if (ret.second == false) {
		throw ReasonError{
			loc,
			"member already in this object",
			{{ret.first->second.get_location(), "first defined here"}}
		};
	}

	return ret.first->second;
}


ObjectInfo::member_info_t &ObjectInfo::get_members() {
	return this->member_info;
}

const ObjectInfo::member_info_t &ObjectInfo::get_members() const {
	return this->member_info;
}


const MemberInfo *ObjectInfo::get_member(const memberid_t &name) const {
	auto it = this->member_info.find(name);

	if (it == std::end(this->member_info)) {
		return nullptr;
	}

	return &it->second;
}


bool ObjectInfo::is_patch() const {
	return this->patch_info.get() != nullptr;
}


bool ObjectInfo::is_initial_patch() const {
	return this->initial_patch;
}


PatchInfo &ObjectInfo::add_patch(const std::shared_ptr<PatchInfo> &info, bool initial) {
	this->initial_patch = initial;
	this->patch_info = info;
	return *this->patch_info.get();
}


const std::shared_ptr<PatchInfo> &ObjectInfo::get_patch() const {
	return this->patch_info;
}


void ObjectInfo::add_inheritance_change(InheritanceChange &&change) {
	this->inheritance_change.push_back(std::move(change));
}


const std::vector<InheritanceChange> &ObjectInfo::get_inheritance_change() const {
	return this->inheritance_change;
}


void ObjectInfo::set_linearization(std::vector<fqon_t> &&lin) {
	this->initial_linearization = std::move(lin);
}


const std::vector<fqon_t> &ObjectInfo::get_linearization() const {
	return this->initial_linearization;
}


void ObjectInfo::set_children(std::unordered_set<fqon_t> &&children) {
	this->initial_children = std::move(children);
}


const std::unordered_set<fqon_t> &ObjectInfo::get_children() const {
	return this->initial_children;
}


std::string ObjectInfo::str() const {
	std::ostringstream builder;

	builder << "ObjectInfo";

	if (this->is_patch()) {
		builder << " " << this->patch_info->str();
	}

	if (this->inheritance_change.size() > 0) {
		builder << " [";

		bool liststart = true;
		for (auto &change : this->inheritance_change) {
			if (not liststart) {
				builder << ", ";
			} else {
				liststart = false;
			}

			switch (change.get_type()) {
			case inher_change_t::ADD_FRONT:
				builder << change.get_target() << "+";
				break;

			case inher_change_t::ADD_BACK:
				builder << "+" << change.get_target();
				break;

			default:
				throw InternalError{"unknown inheritance change type"};
			}
		}

		builder << "]";
	}

	builder << ":" << std::endl;

	if (this->member_info.size() == 0) {
		builder << " [no members]" << std::endl;
	}

	for (auto &it : this->member_info) {
		const auto &memberid = it.first;
		const auto &memberinfo = it.second;

		builder << " -> " << memberid;
		builder << " : " << memberinfo.str() << std::endl;
	}

	return builder.str();
}

} // namespace nyan
