// Copyright 2020-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <unordered_map>

#include "../api_error.h"
#include "../compiler.h"
#include "../util.h"
#include "container.h"
#include "value.h"


namespace nyan {

/**
 * Nyan value to store a dict/map/assocated array of things.
 */
class Dict : public Value {
public:
	using value_storage = std::unordered_map<ValueHolder,ValueHolder>;
	using key_type = typename value_storage::key_type;
	using element_type = typename value_storage::value_type;
	using value_const_iterator = typename value_storage::const_iterator;

	Dict();
	Dict(std::unordered_map<ValueHolder,ValueHolder> &&values);


	size_t hash() const override {
		throw APIError{"Dicts are not hashable."};
	}


	size_t size() const {
		return this->values.size();
	}


	void clear() {
		this->values.clear();
	}


	const value_storage &get() const {
		return this->values;
	}

	ValueHolder copy() const override;
	std::string str() const override;
	std::string repr() const override;

	const std::unordered_set<nyan_op> &allowed_operations(const Type &with_type) const override;
	const BasicType &get_type() const override;

protected:
	void apply_value(const Value &value, nyan_op operation) override;


	bool equals(const Value &other) const override {
        auto &other_val = dynamic_cast<const Dict &>(other);

        return values == other_val.values;
    }

	/**
	 * Dict value storage (this is an unordered map).
	 */
	value_storage values;
};

} // namespace nyan
