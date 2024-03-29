// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <string>

#include "value.h"


namespace nyan {

class IDToken;


/**
 * Nyan value to store file names as nyan values.
 */
class Filename : public Value {
public:
	Filename(const std::string &path);
	Filename(const IDToken &token);

	const std::string &get() const;

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;
	size_t hash() const override;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

protected:
	bool apply_value(const Value &value, nyan_op operation) override;
	bool equals(const Value &other) const override;

	std::string path;
};

} // namespace nyan
