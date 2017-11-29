// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>


namespace nyan {

class Value;


/**
 * Wrapper class to hold values by shared pointer.
 * Used to redirect the hashing and comparison function inside the ptr.
 */
class ValueHolder {
public:
	ValueHolder();
	ValueHolder(std::shared_ptr<Value> &&value);
	ValueHolder(const std::shared_ptr<Value> &value);

	Value *get_value() const;
	const std::shared_ptr<Value> &get_ptr() const;
	void clear();
	bool exists() const;

	Value &operator *() const;
	Value *operator ->() const;
	bool operator ==(const ValueHolder &other) const;
	bool operator !=(const ValueHolder &other) const;

protected:
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
	size_t operator ()(const nyan::ValueHolder &val) const;
};

} // namespace std

