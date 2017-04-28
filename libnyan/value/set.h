// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <unordered_set>

#include "../ops.h"
#include "set_base.h"


namespace nyan {

/**
 * Value to store a unordered set of things.
 */
class Set
	: public SetBase<std::unordered_set<ValueContainer>> {

	// fetch the constructors
	using SetBase<
		std::unordered_set<ValueContainer>>::SetBase;

public:
	Set();
	Set(std::vector<ValueContainer> &values);

	std::string str() const override;
	std::string repr() const override;

	std::unique_ptr<Value> copy() const override;

	bool add(ValueContainer &&value) override;
	bool contains(Value *value) override;
	bool remove(Value *value) override;

	const std::unordered_set<nyan_op> &allowed_operations(nyan_basic_type value_type) const override;
	const nyan_basic_type &get_type() const override;

protected:
	void apply_value(const Value *value, nyan_op operation) override;
};

} // namespace nyan
