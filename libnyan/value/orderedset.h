// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_ORDEREDSET_H_
#define NYAN_NYAN_VALUE_ORDEREDSET_H_

#include "set_base.h"

#include "../ops.h"
#include "../datastructure/orderedset.h"


namespace nyan {

/**
 * Nyan value to store an ordered set of things.
 */
class OrderedSet
	: public SetBase<datastructure::OrderedSet<ValueContainer>> {

	using SetBase<datastructure::OrderedSet<ValueContainer>>::SetBase;

public:
	OrderedSet();
	OrderedSet(std::vector<ValueContainer> &values);

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

#endif
