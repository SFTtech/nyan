// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>

#include "value.h"
#include "../id_token.h"


namespace nyan {

/**
 * Nyan value to store file names as nyan values.
 */
class Filename : public Value {
public:
	Filename(const std::string &path);
	Filename(const IDToken &token);

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

protected:
	void apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override;

	std::string path;
};

} // namespace nyan
