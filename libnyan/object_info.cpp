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
	location{location} {}


const Location &ObjectInfo::get_location() const {
	return this->location;
}


MemberInfo &ObjectInfo::add_member(const memberid_t &name,
                                   MemberInfo &&member) {

	// copy the location so it's still valid if the insert fails.
	Location loc = member.get_location();

	auto ret = this->member_info.insert({name, std::move(member)});
	if (ret.second == false) {
		throw FileError{loc, "member already in this object"};
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


void ObjectInfo::add_inheritance_add(fqon_t &&name) {
	this->inheritance_add.push_back(std::move(name));
}


const std::vector<fqon_t> ObjectInfo::get_inheritance_add() const {
	return this->inheritance_add;
}


std::string ObjectInfo::str() const {
	std::ostringstream builder;

	builder << "ObjectInfo";

	if (this->is_patch()) {
		builder << " " << this->patch_info->str();
	}

	if (this->inheritance_add.size() > 0) {
		builder << " [+";
		builder << util::strjoin(
			"+, ",
			this->inheritance_add
		);

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
