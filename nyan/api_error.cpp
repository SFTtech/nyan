// Copyright 2019-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "api_error.h"

#include <sstream>


namespace nyan {

APIError::APIError(const std::string &msg)
    :
    Error{msg} {}


InvalidObjectError::InvalidObjectError()
    :
    APIError("uninitialized object was used") {}


MemberTypeError::MemberTypeError(const fqon_t &objname, const memberid_t &member,
                                 const std::string &real_type, const std::string &wrong_type)
    :
    APIError{
    (static_cast<const std::ostringstream&>(
        std::ostringstream{} << "type mismatch for member " << objname + "." << member
        << ": tried to convert real type " << real_type << " to " << wrong_type)
    ).str()},
    objname{objname},
    member{member},
    real_type{real_type},
    wrong_type{wrong_type} {}


ObjectNotFoundError::ObjectNotFoundError(const fqon_t &obj_name)
    :
    APIError{"object not found: " + obj_name},
    objname{obj_name} {}


MemberNotFoundError::MemberNotFoundError(const fqon_t &obj_name,
                                         const memberid_t &member_name)
    :
    APIError{"Could not find member " + obj_name + "." + member_name},
    objname{obj_name},
    name{member_name} {}

} // namespace nyan
