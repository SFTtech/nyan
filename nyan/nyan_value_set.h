// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_VALUE_SET_H_
#define NYAN_NYAN_VALUE_SET_H_

#include <unordered_set>
#include <vector>

#include "nyan_container.h"
#include "nyan_ptr_container.h"
#include "nyan_value.h"
#include "nyan_value_container.h"


namespace nyan {

/**
 * Nyan value to store a set of things.
 * TODO operations with swap!
 */
class NyanSet : public NyanContainer {
public:
	NyanSet();
	NyanSet(std::vector<NyanValueContainer> &values);
	NyanSet(const NyanSet &other);

	std::unique_ptr<NyanValue> copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	bool add(NyanValueContainer &&value) override;
	bool contains(NyanValue *value) override;
	bool remove(NyanValue *value) override;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	bool equals(const NyanValue &other) const override;
	const std::unordered_set<nyan_op> &allowed_operations(nyan_type value_type) const override;

	std::unordered_set<NyanValueContainer> values;
};

} // namespace nyan

#endif
