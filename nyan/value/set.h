// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <unordered_set>

#include "../ops.h"
#include "set_base.h"


namespace nyan {

/** datatype used for ordered set storage */
using set_t = std::unordered_set<ValueHolder>;


/**
 * Value to store a unordered set of things.
 */
class Set
	: public SetBase<set_t> {

	// fetch the constructors
	using SetBase<set_t>::SetBase;

public:
	Set();
	Set(std::vector<ValueHolder> &&values);

	std::string str() const override;
	std::string repr() const override;

	ValueHolder copy() const override;

	bool add(const ValueHolder &value) override;
	bool contains(const ValueHolder &value) const override;
	bool remove(const ValueHolder &value) override;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;
};

} // namespace nyan
