// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>

#include "value_wrapper.h"


namespace nyan {

/**
 * Wrapper class to hold values.
 * Used to redirect the hashing and comparison function inside the ptr.
 */
class ValueHolder : public ValueWrapper {
public:
	ValueHolder();
	ValueHolder(std::shared_ptr<Value> &&value);
	ValueHolder(const std::shared_ptr<Value> &value);

	virtual Value *get_value() const override;
	void clear() override;
	bool exists() const override;

protected:
	std::shared_ptr<Value> value;
};

} // namespace nyan


namespace std {

template <>
struct hash<nyan::ValueHolder> {
	size_t operator ()(const nyan::ValueHolder &val) const;
};

} // namespace std

