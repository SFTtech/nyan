// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>

#include "ops.h"
#include "type.h"
#include "value/value.h"

namespace nyan {


/**
 * Stores a member of a Object.
 * Also responsible for validating applied operators.
 */
class Member {
public:
	/**
	 * Member with value.
	 */
	Member(override_depth_t depth,
	       nyan_op operation,
	       ValueHolder &&value);

	Member(const Member &other);
	Member(Member &&other) noexcept;
	Member &operator =(const Member &other);
	Member &operator =(Member &&other) noexcept;

	~Member() = default;

	/**
	 * Provide the operation stored in the member.
	 */
	nyan_op get_operation() const;

	/**
	 * Return the value stored in this member.
	 */
	const Value &get_value() const;

	/**
	 * Apply another member to this one.
	 * This applies the member with its operation
	 * to this member.
	 */
	void apply(const Member &change);

	/**
	 * Save a value to the cache.
	 */
	void cache_save(ValueHolder &&value);

	/**
	 * String representation of this member.
	 */
	std::string str() const;

protected:

	/**
	 * Number of @ chars before the operation,
	 * those define the override depth when applying the patch.
	 */
	override_depth_t override_depth = 0;

	/**
	 * operation specified for this member.
	 */
	nyan_op operation = nyan_op::INVALID;

	/**
	 * Value to cache the calculation result.
	 * It stores the result of the application of all operations on
	 * the inheritance tree.
	 */
	ValueHolder cached_value;

	/**
	 * Value stored in this member.
	 */
	ValueHolder value;
};


} // namespace nyan
