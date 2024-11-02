// Copyright 2018-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include "../id_token.h"
#include "value.h"


namespace nyan {

/**
 * Nyan value to store a boolean value (true/false).
 */
class Boolean : public Value {
public:
	Boolean(const bool &value);
	Boolean(const IDToken &token);

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	bool get() const {
		return *this;
	}

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

	operator bool() const {
		return this->value;
	}

protected:
	bool apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override;

	bool value;
};

} // namespace nyan
