// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>


namespace nyan {

class Value;


/**
 * Wrapper class to hold values by shared pointer.
 * Used to redirect the hashing and comparison function inside the ptr.
 * The shared_ptr must be initialized when using ValueHolder.
 */
class ValueHolder {
public:
	ValueHolder();
	ValueHolder(std::shared_ptr<Value> &&value);
	ValueHolder(const std::shared_ptr<Value> &value);

	/**
	 * Assign a new value to the holder.
	 */
	ValueHolder &operator=(const std::shared_ptr<Value> &value);

	/**
	 * Get the shared pointer of this ValueHolder.
	 *
	 * @return Shared pointer to this holder's value.
	 */
	const std::shared_ptr<Value> &get_ptr() const;

	/**
	 * Check if this holder points to a value.
	 *
	 * @return true if \p this->value is not nullptr, else false.
	 */
	bool exists() const;

	/**
	 * Get the value stored at this holder's pointer.
	 *
	 * @return Value stored at this holder's pointer.
	 */
	Value &operator*() const;

	/**
	 * Get the shared pointer of this ValueHolder.
	 *
	 * @return Shared pointer to this holder's value.
	 */
	Value *operator->() const;

	/**
	 * Compare two ValueHolders for equality.
	 *
	 * @return true if the values stored by the holders are equal, else false.
	 */
	bool operator==(const ValueHolder &other) const;

	/**
	 * Compare two ValueHolders for inequality.
	 *
	 * @return true if the values stored by the holders are not equal, else false.
	 */
	bool operator!=(const ValueHolder &other) const;

protected:
	/**
	 * Shared pointer to the wrapped value.
	 */
	std::shared_ptr<Value> value;
};

} // namespace nyan


namespace std {

/**
 * Hashing for ValueHolders.
 * Relays the hash to the internally stored value.
 */
template <>
struct hash<nyan::ValueHolder> {
	size_t operator()(const nyan::ValueHolder &val) const;
};

} // namespace std
