// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_ORDEREDSET_H_
#define NYAN_NYAN_VALUE_ORDEREDSET_H_

#include "nyan_ops.h"
#include "nyan_orderedset.h"
#include "nyan_value_set_base.h"


namespace nyan {

/**
 * Nyan value to store an ordered set of things.
 */
class NyanOrderedSet
	: public NyanSetBase<OrderedSet<NyanValueContainer>> {

	using NyanSetBase<OrderedSet<NyanValueContainer>>::NyanSetBase;

public:
	NyanOrderedSet();
	NyanOrderedSet(std::vector<NyanValueContainer> &values);

	std::string str() const override;
	std::string repr() const override;

	std::unique_ptr<NyanValue> copy() const override;

	bool add(NyanValueContainer &&value) override;
	bool contains(NyanValue *value) override;
	bool remove(NyanValue *value) override;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	const std::unordered_set<nyan_op> &allowed_operations(nyan_type value_type) const override;

};

} // namespace nyan

#endif
