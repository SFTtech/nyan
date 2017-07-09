// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <functional>
#include <memory>


namespace nyan {

class Value;


/**
 * Wrapper class to for values.
 * Used to redirect the hashing and comparison function inside the actual holder.
 */
class ValueWrapper {
public:
	ValueWrapper();
	virtual ~ValueWrapper();

	virtual Value *get_value() const = 0;
	virtual void clear() = 0;
	virtual bool exists() const = 0;

	Value &operator *() const;
	Value *operator ->() const;
	bool operator ==(const ValueWrapper &other) const;
	bool operator !=(const ValueWrapper &other) const;
};

} // namespace nyan


namespace std {

template <>
struct hash<nyan::ValueWrapper> {
	size_t operator ()(const nyan::ValueWrapper &val) const;
};

} // namespace std
