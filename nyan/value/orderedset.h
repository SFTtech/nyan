// Copyright 2016-2023 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include "set_base.h"
#include "container_types.h"

#include "../ops.h"
#include "value_holder.h"


namespace nyan {

class Value;


/**
 * Nyan value to store an ordered set of things.
 */
class OrderedSet
	: public SetBase<ordered_set_t> {

	// fetch the constructors
	using SetBase<ordered_set_t>::SetBase;

public:
	OrderedSet();
	OrderedSet(std::vector<ValueHolder> &&values);

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
