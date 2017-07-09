// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "config.h"
#include "value.h"


namespace nyan {

/**
 * Nyan value to store object references.
 * They are stored just by remembering the fully-qualified object name.
 */
class ObjectValue : public Value {
public:
	ObjectValue(const fqon_t &name);

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	/** return the stored fqon */
	const fqon_t &get() const;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

protected:
	void apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override;

	fqon_t name;
};

} // namespace nyan
