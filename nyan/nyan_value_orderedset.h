// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_ORDEREDSET_H_
#define NYAN_NYAN_VALUE_ORDEREDSET_H_

#include <list>
#include <unordered_set>
#include <vector>

#include "nyan_container.h"
#include "nyan_orderedset.h"
#include "nyan_ptr_container.h"
#include "nyan_value.h"
#include "nyan_value_container.h"


namespace nyan {


/**
 * Nyan value to store an ordered set of things.
 */
class NyanOrderedSet : public NyanContainer {
public:
	NyanOrderedSet();
	NyanOrderedSet(const std::vector<NyanToken> &tokens);

	std::unique_ptr<NyanValue> copy() const override;
	std::string str() const override;
	size_t hash() const override;

	bool add(NyanValueContainer &&value) override;
	bool contains(NyanValue *value) override;
	bool remove(NyanValue *value) override;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	bool equals(const NyanValue &other) const override;
	const std::unordered_set<nyan_op> &allowed_operations(nyan_type value_type) const override;

	/**
	 * Data storage.
	 */
	OrderedSet<NyanValueContainer> values;
};

} // namespace nyan

#endif
