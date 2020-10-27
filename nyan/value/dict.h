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
	using value_type = typename value_storage::mapped_type;
	using element_type = typename value_storage::value_type;
	using value_const_iterator = typename value_storage::const_iterator;

	using iterator = ContainerIterator<std::pair<Value,Value>>;
	using const_iterator = ContainerIterator<const std::pair<Value,Value>>;

	using holder_iterator = ContainerIterator<element_type>;
	using holder_const_iterator = ContainerIterator<const element_type>;

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

	iterator begin() {
		throw Error{
			"Dicts are only const-iterable. "
			"make it const by using e.g. "
			"for (auto &it = std::as_const(dict))"
		};
	}

	iterator end() {
		// also throw the error above.
		return this->begin();
	}


	holder_iterator values_begin() {
		throw Error{
			"Dict values holders are not non-const-iterable."
		};
	}


	holder_iterator values_end() {
		// also throw the error above.
		return this->values_begin();
	}


	/**
	 * Get an iterator to the underlying dict storage.
	 * Contrary to the above, this will allow to get the
	 * ValueHolders.
	 */
	holder_const_iterator values_begin() const {
		auto real_iterator = std::make_unique<
			DefaultIterator<value_const_iterator,
			                holder_const_iterator::elem_type>>(
				                std::begin(this->values)
			                );

		return holder_const_iterator{std::move(real_iterator)};
	}

	/**
	 * Iterator to end of the underlying storage.
	 */
	holder_const_iterator values_end() const {
		auto real_iterator = std::make_unique<
		DefaultIterator<value_const_iterator,
		                holder_const_iterator::elem_type>>(
			                std::end(this->values)
		                );

		return holder_const_iterator{std::move(real_iterator)};
	}


	bool add(const element_type &value);
	bool contains(const key_type &value) const;
	bool remove(const key_type &value);

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
