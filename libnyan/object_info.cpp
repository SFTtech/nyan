// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object_info.h"

#include <sstream>

#include "error.h"
#include "util.h"


namespace nyan {

ObjectInfo::ObjectInfo(const Location &location)
	:
	location{location} {}


MemberInfo &ObjectInfo::add_member(const fqon_t &name, MemberInfo &&member) {
	auto ret = this->member_info.insert({name, std::move(member)});
	if (ret.second == false) {
		throw Error{name + ": member already known"};
	}

	return ret.first->second;
}


ObjectInfo::member_info_t &ObjectInfo::get_members() {
	return this->member_info;
}


const MemberInfo *ObjectInfo::get_member(const memberid_t &name) const {
	auto it = this->member_info.find(name);

	if (it == std::end(this->member_info)) {
		return nullptr;
	}

	return &it->second;
}


void ObjectInfo::set_target(const fqon_t &name) {
	this->target = name;
	this->is_patch = true;
}


void ObjectInfo::add_inheritance_add(const fqon_t &name) {
	this->inheritance_add.push_back(name);
}


std::string ObjectInfo::str() const {
	std::ostringstream builder;

	builder << "ObjectInfo";

	if (this->target.size() > 0) {
		builder << " <" << this->target << ">";
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
