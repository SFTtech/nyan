// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_OBJECT_H_
#define NYAN_NYAN_OBJECT_H_

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "nyan_member.h"

namespace nyan {

class NyanStore;

/**
 * Data definition with members and inheritance.
 */
class NyanObject : public NyanMember {
public:
	NyanObject(NyanStore *store);
	virtual ~NyanObject() = default;

	/**
	 * Return the name of this object.
	 */
	const std::string &get_name() const;

	/**
	 * Get a member value of this object.
	 */
	virtual const NyanObject *get(const std::string &member);

	/**
	 * Patch this object with a patch object.
	 */
	virtual void patch(const NyanObject *top);

	/**
	 * Cast this object to another type name.
	 * This creates a non-registered NyanObject to view this object
	 * as the target type.
	 */
	NyanObject cast(const std::string &type_name);

	/**
	 * Creates a non-registered NyanObject that
	 * has a precalculated member list and member values.
	 * This can be used to create a snapshot of some object.
	 */
	NyanObject bake(const std::string &new_name);

	/**
	 * Implements the C3 multi inheritance linearization algorithm.
	 */
	std::vector<NyanObject *> &linearize(std::unordered_set<NyanObject *> &seen);

	/**
	 * Return a string representation of this object.
	 */
	virtual std::string str();

protected:
	std::string name;
	std::vector<NyanObject *> parents;
	std::unordered_map<std::string, std::unique_ptr<NyanMember>> members;

	NyanStore *store;
};


} // namespace nyan

#endif
