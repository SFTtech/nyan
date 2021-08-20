// Copyright 2020-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "value.h"


namespace nyan {


/**
 * Nyan value to store None (basically a "no value" placeholder).
 */
class None : public Value {
public:
	None();

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

	/** the global None value */
	static std::shared_ptr<None> value;

protected:
	bool apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override;
};

} // namespace nyan
