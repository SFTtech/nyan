// Copyright 2019-2019 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "error.h"


namespace nyan {

/**
 * Error thrown when the API user most likely did something wrong.
 */
class APIError : public Error {
public:
	APIError(const std::string &msg);
};


/**
 * An Object was not initialized properly.
 * This happens when you used the default constructor without
 * updating the Object from a View.
 */
class InvalidObjectError : public APIError {
public:
	InvalidObjectError();
};


/**
 * The type of a member was queried wrongly.
 */
class MemberTypeError : public APIError {
public:
	MemberTypeError(const fqon_t &objname, const memberid_t &member,
	                const std::string &real_type, const std::string &wrong_type);

protected:
	fqon_t name;
	memberid_t member;
	std::string real_type;
	std::string wrong_type;
};


/**
 * An object queried over the API is not found.
 */
class ObjectNotFoundError : public APIError {
public:
	ObjectNotFoundError(const fqon_t &objname);

protected:
	fqon_t name;
};


/**
 * An object member queried over the API is not found.
 */
class MemberNotFoundError : public APIError {
public:
	MemberNotFoundError(const fqon_t &objname,
	                    const memberid_t &membername);

protected:
	fqon_t obj_name;
	memberid_t name;
};

} // namespace nyan
