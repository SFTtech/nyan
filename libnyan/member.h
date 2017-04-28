// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_MEMBER_H_
#define NYAN_NYAN_MEMBER_H_

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>

#include "location.h"
#include "ops.h"
#include "type.h"
#include "type_container.h"
#include "value/value.h"
#include "value/container.h"

namespace nyan {

class Object;

/**
 * Stores a member of a Object.
 * Also responsible for validating applied operators.
 */
class Member {
public:
	/**
	 * Member without value.
	 */
	Member(const Location &location, TypeContainer &&type);

	/**
	 * Member with value.
	 */
	Member(const Location &location,
	           TypeContainer &&type, nyan_op operation,
	           ValueContainer &&value);

	Member(Member &&other) noexcept;
	const Member &operator =(Member &&other) noexcept;

	Member(const Member &other) = delete;
	const Member &operator =(const Member &other) = delete;

	virtual ~Member() = default;

	/**
	 * String representation of this member.
	 */
	std::string str() const;

	/**
	 * Return the value of this member.
	 * @returns nullptr if there is no value yet.
	 */
	Value *get_value_ptr() const;

	/**
	 * Get a member value and cast the result to the
	 * specified output type.
	 */
	template <typename T>
	T *get_value() const {
		static_assert(std::is_base_of<Value, T>::value,
		              "only nyan value types are supported");

		return dynamic_cast<T *>(this->get_value_ptr());
	}

	/**
	 * Return the type of this member.
	 */
	Type *get_type() const;

	/**
	 * Provide the operation stored in the member.
	 */
	nyan_op get_operation() const;

	/**
	 * Save a previous result of `get_value` to bypass calculation
	 * next time.
	 */
	void cache_save(std::unique_ptr<Value> &&value);

	/**
	 * Return the content of the value calculation cache.
	 * nullptr if the cache is empty.
	 */
	Value *cache_get() const;

	/**
	 * Clear the value calculation cache.
	 */
	void cache_reset();

	/**
	 * Get the location where this member was defined.
	 */
	const Location &get_location() const;

protected:
	/**
	 * The type of this member.
	 * Either, this member defines the type, or it points
	 * to the definition at another member.
	 */
	TypeContainer type;

	/**
	 * operation specified for this member.
	 */
	nyan_op operation;

	/**
	 * Value to cache the calculation result.
	 * It stores the result of the application of all operations on
	 * the inheritance tree.
	 */
	std::unique_ptr<Value> cached_value;

	/**
	 * Value of just this member.
	 */
	ValueContainer value;

	/**
	 * Location where this member was defined.
	 */
	Location location;
};


} // namespace nyan

#endif
