// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#include "member_info.h"

#include <sstream>

#include "type.h"


namespace nyan {

MemberInfo::MemberInfo(const Location &location)
    :
    location{location},
    initial_def{false} {}


Type &MemberInfo::set_type(std::shared_ptr<Type> &&type, bool initial) {
    this->initial_def = initial;
    this->type = std::move(type);
    return *this->type.get();
}


Type &MemberInfo::set_type(const std::shared_ptr<Type> &type, bool initial) {
    this->initial_def = initial;
    this->type = type;
    return *this->type.get();
}


const std::shared_ptr<Type> &MemberInfo::get_type() const {
    return this->type;
}


const Location &MemberInfo::get_location() const {
    return this->location;
}


bool MemberInfo::is_initial_def() const {
    return this->initial_def;
}


std::string MemberInfo::str() const {
    std::ostringstream builder;

    if (this->type) {
        builder << this->type->str();
    }
    else {
        builder << "[no type]";
    }

    return builder.str();
}

} // namespace nyan
