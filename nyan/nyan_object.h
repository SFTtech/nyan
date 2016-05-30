// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_OBJECT_H_
#define NYAN_NYAN_OBJECT_H_

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "nyan_location.h"
#include "nyan_member.h"
#include "nyan_value.h"

namespace nyan {

class NyanParser;
class NyanStore;

/**
 * Data definition with members and inheritance.
 */
class NyanObject : public NyanValue {
	friend class NyanParser;

public:
	NyanObject(const NyanLocation &location, NyanStore *store=nullptr);
	virtual ~NyanObject() = default;

	/**
	 * Whether the object is registered in a store.
	 */
	bool is_registered() const;

	/**
	 * Return the name of this object.
	 */
	const std::string &get_name() const;

	/**
	 * Get a member of this object.
	 * throws NameError if the member doesn't exist.
	 */
	const NyanMember *get_member(const std::string &member) const;

	/**
	 * Get a member value of this object.
	 */
	virtual const NyanValue &get(const std::string &member);

	/**
	 * Get the type of some member.
	 */
	virtual const NyanType &get_type(const std::string &member) const;

	/**
	 * Test if this object has a member of given name.
	 */
	virtual bool has(const std::string &member) const;

	/**
	 * Test if this object is a child of the given parent.
	 * Returns true if parent equals this object.
	 */
	virtual bool is_child_of(const NyanObject *parent) const;

	/**
	 * Patch this object with a patch object.
	 */
	void patch(const NyanObject *top);

	/**
	 * Check if this object is a patch.
	 */
	bool is_patch() const;

	/**
	 * Return a copy of this NyanObject.
	 */
	std::unique_ptr<NyanValue> copy() const override;

	/**
	 * Cast this object to another type name.
	 * This creates a non-registered NyanObject to view this object
	 * as the target type.
	 */
	NyanObject cast(const std::string &type_name) const;

	/**
	 * Creates a non-registered NyanObject that
	 * has a precalculated member list and member values.
	 * This can be used to create a snapshot of some object.
	 */
	NyanObject bake(const std::string &new_name) const;

	/**
	 * Provide the linearization of this object.
	 * Will return an empty vector if the linearization was not
	 * generated before.
	 */
	const std::vector<NyanObject *> &get_linearization() const;

	/**
	 * Invokes the C3 multi inheritance linearization to determine
	 * the "right" parent order.
	 */
	const std::vector<NyanObject *> &generate_linearization();

	/**
	 * Remove the calculated multi inheritance linearization.
	 * This needs to be done whenever a patch is applied
	 * that modifies the inheritance of this object.
	 */
	void delete_linearization();

	/**
	 * Return a full string representation of this object.
	 */
	std::string str() const override;

	/**
	 * Return a smaller string representation of this object.
	 */
	std::string repr() const override;

	/**
	 * Hash function for the object.
	 * An object is uniquely identified by its name.
	 */
	size_t hash() const override;

protected:
	/**
	 * Implements the C3 multi inheritance linearization algorithm
	 * to bring the parents of this object into the "right" order.
	 */
	const std::vector<NyanObject *> &linearize_walk(std::unordered_set<NyanObject *> &seen);

	/**
	 * Get the member entry of this object.
	 *
	 * @returns nullptr if the member was not found.
	 */
	virtual const NyanMember *get_member_ptr(const std::string &member) const;
	/**
	 * Get the member entry of this object and allow write access to it.
	 *
	 * @returns nullptr if the member was not found.
	 */
	virtual NyanMember *get_member_ptr_rw(const std::string &member);

	/**
	 * Determine the most specialized type as set by parents for the
	 * given member name.
	 * @returns nullptr if it could not be inferred
	 */
	NyanType *infer_type(const std::string &member) const;

	/**
	 * Apply changes in an NyanObject to this NyanObject.
	 * This basically updates the values and may add members.
	 * This does the real patching work.
	 */
	void apply_value(const NyanValue *value, nyan_op operation) override;

	/**
	 * Comparison for two NyanObjects.
	 */
	bool equals(const NyanValue &other) const override;

	/**
	 * Allowed operations for a NyanObject.
	 */
	const std::unordered_set<nyan_op> &allowed_operations(nyan_type value_type) const override;

	/**
	 * Where this object was created.
	 */
	NyanLocation location;

	/**
	 * The name of this NyanObject.
	 */
	std::string name;

	/**
	 * Parent NyanObjects to inherit from.
	 */
	std::vector<NyanObject *> parents;

	/**
	 * Member variables of this object.
	 * This is the main key->value store.
	 * Soooo much daaataaaa!
	 */
	std::unordered_map<std::string, NyanMember> members;

	/**
	 * The storage this object is associated to.
	 */
	NyanStore *store;

	/**
	 * all linearized parents.
	 */
	std::vector<NyanObject *> linearization;
};


} // namespace nyan

#endif
