// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_MEMBER_H_
#define NYAN_NYAN_MEMBER_H_

#include <string>
#include <unordered_set>

#include "nyan_ops.h"

namespace nyan {

class NyanObject;

/**
 * Stores a member of a NyanObject.
 */
class NyanMember {
	friend class NyanObject;

public:
	NyanMember();
	virtual ~NyanMember();

	virtual std::string str();

protected:
	std::unordered_set<nyan_op> operations;
	NyanObject *object;
};


/**
 * Nyan value to store a member that doesn't have a value yet.
 */
class NyanNone : public NyanMember {
public:
	NyanNone();
	virtual ~NyanNone();
};


/**
 * Nyan value to store text.
 */
class NyanText : public NyanMember {
public:
	NyanText();
	virtual ~NyanText();
};


/**
 * Nyan value to store a filename member which preserves the relative
 * location to the nyan file it was defined in.
 */
class NyanFilename : public NyanMember {
public:
	NyanFilename();
	virtual ~NyanFilename();
};


/**
 * Nyan value to store a number.
 */
class NyanInt : public NyanMember {
public:
	NyanInt();
	virtual ~NyanInt();
};


} // namespace nyan

#endif
