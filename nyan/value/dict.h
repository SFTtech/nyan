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
 * Container iterator for the Set.
 *
 * Used for walking over the contained values,
 * i.e. it unpacks the ValueHolders!
 *
 */
template<typename iter_type, typename elem_type>
class DictIterator : public ContainerIterBase<elem_type> {
public:
	using this_type = DictIterator<iter_type, elem_type>;
	using base_type = ContainerIterBase<elem_type>;

	explicit DictIterator(iter_type &&iter)
		:
		iterator{std::move(iter)} {}

	/**
	 * Advance the iterator to the next element in the dict.
	 */
	base_type &operator ++() override {
		++this->iterator;
		return *this;
	}

	/**
	 * Get the element the iterator is currently pointing to.
	 */
	elem_type &operator *() const override {
		// unpack the ValueHolder!
		return *(*this->iterator);
	}

protected:
	/**
	 * compare two iterators
	 */
	bool equals(const base_type &other) const override {
		auto other_me = dynamic_cast<const this_type &>(other);
		return (this->iterator == other_me.iterator);
	}

	/**
	 * The wrapped std::iterator, from the Set std::unordered_set.
	 */
	iter_type iterator;
};


/**
 * Nyan value to store a dict/map/assocated array of things.
 *
 * T is the underlying storage type to store the Values.
 */
class Dict : Value {
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
