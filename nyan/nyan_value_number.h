// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_NUMBER_H_
#define NYAN_NYAN_VALUE_NUMBER_H_

#include "nyan_value.h"


namespace nyan {

/**
 * Nyan value to store a number.
 */
template <typename T>
class NyanNumber : public NyanValue {
public:
	NyanNumber(const NyanToken &token);
	NyanNumber(T value);

	std::unique_ptr<NyanValue> copy() const override;
	std::string str() const override;
	size_t hash() const override;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	bool equals(const NyanValue &other) const override;
	const std::unordered_set<nyan_op> &allowed_operations() const override;

	T value;
};


/**
 * Integer storage.
 */
using NyanInt = NyanNumber<int64_t>;


/**
 * Floating point data type.
 */
using NyanFloat = NyanNumber<double>;

} // namespace std

#endif
