// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_OBJECT_H_
#define NYAN_NYAN_OBJECT_H_

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "location.h"
#include "member.h"
#include "value/value.h"

namespace nyan {


class Database;
class Object;
class Parser;


/**
 * Data definition with members and inheritance.
 */
class Object : public Value {
	friend class Parser;

public:
	Object(const Location &location, Database *database);
	virtual ~Object() = default;

	/**
	 * Whether the object is registered in a database.
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
	const Member *get_member(const std::string &member) const;

	/**
	 * Get a member value of this object.
	 */
	virtual const Value &get(const std::string &member);

	/**
	 * Get the type of some member.
	 */
	virtual const Type &get_member_type(const std::string &member) const;

	/**
	 * Test if this object has a member of given name.
	 */
	virtual bool has(const std::string &member) const;

	/**
	 * Test if this object is a child of the given parent.
	 * Returns true if parent equals this object.
	 */
	virtual bool is_child_of(const Object *parent) const;

	/**
	 * Patch this object with a patch object.
	 */
	void patch(const Object *top);

	/**
	 * Check if this object is a patch.
	 */
	bool is_patch() const;

	/**
	 * Return a copy of this Object.
	 */
	std::unique_ptr<Value> copy() const override;

	/**
	 * Creates a copy of this Object that
	 * has a precalculated member list and member values.
	 *
	 * The returned pointer is the baked version of this
	 *
	 * This can be used to create a snapshot copy of some object.
	 */
	Object *baked_copy(const std::string &new_name) const;

	/**
	 * Save ("bake") all values of this Object,
	 * and create a exact copy in a new  TODO
	 *
	 */
	void bake(const std::string &new_name);

	/**
	 * Provide the linearization of this object.
	 * Will return an empty vector if the linearization was not
	 * generated before.
	 */
	const std::vector<Object *> &get_linearization() const;

	/**
	 * Invokes the C3 multi inheritance linearization to determine
	 * the "right" parent order.
	 */
	const std::vector<Object *> &generate_linearization();

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
	const std::vector<Object *> &linearize_walk(std::unordered_set<Object *> &seen);

	/**
	 * Get the member entry of this object.
	 *
	 * @returns nullptr if the member was not found.
	 */
	virtual const Member *get_member_ptr(const std::string &member) const;
	/**
	 * Get the member entry of this object and allow write access to it.
	 *
	 * @returns nullptr if the member was not found.
	 */
	virtual Member *get_member_ptr_rw(const std::string &member);

	/**
	 * Determine the most specialized type as set by parents for the
	 * given member name.
	 * @returns nullptr if it could not be inferred
	 */
	Type *infer_type(const std::string &member) const;

	/**
	 * Apply changes in an Object to this Object.
	 * This basically updates the values and may add members.
	 * This does the real patching work.
	 */
	void apply_value(const Value *value, nyan_op operation) override;

	/**
	 * Comparison for two Objects.
	 */
	bool equals(const Value &other) const override;

	/**
	 * Get the Type of this object.
	 */
	const nyan_basic_type &get_type() const override;

	/**
	 * Allowed operations for a Object.
	 */
	const std::unordered_set<nyan_op> &allowed_operations(nyan_basic_type value_type) const override;

	/**
	 * Where this object was created.
	 */
	Location location;

	/**
	 * The name of this Object.
	 */
	std::string name;

	/**
	 * Parent Objects to inherit from.
	 */
	std::vector<Object *> parents;

	/**
	 * Member variables of this object.
	 * This is the main key->value store.
	 * Soooo much daaataaaa!
	 */
	std::unordered_map<std::string, Member> members;

	/**
	 * The database that stores this object.
	 * nullptr means it is not registered in the storage.
	 */
	Database *database;

	/**
	 * all linearized parents.
	 */
	std::vector<Object *> linearization;
};


} // namespace nyan

#endif
