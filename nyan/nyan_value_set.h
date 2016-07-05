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

class NyanSet;

/**
 * Container iterator for the NyanSet.
 */
template<typename elem_type, typename set_type>
class NyanSetIterator : public ContainerIterBase<elem_type> {
public:

	using this_type = NyanSetIterator<elem_type, set_type>;
	using base_type = ContainerIterBase<elem_type>;

	// magic: determine the iterator type used to access
	//        data in the std::unordered_set of NyanSet.
	using iter_type = typename std::conditional<
		std::is_const<elem_type>::value,
		typename set_type::value_const_iterator,
		typename set_type::value_iterator>::type;

	NyanSetIterator(set_type *set, bool at_start)
		:
		iterator{at_start ? set->values_begin() : set->values_end()} {}

	virtual ~NyanSetIterator() = default;

	/**
	 * Advance the iterator to the next element in the set.
	 */
	base_type &operator ++() override {
		this->iterator++;
		return *this;
	}

	/**
	 * Get the element the iterator is currently pointing to.
	 */
	elem_type &operator *() const override {
		// unpack the NyanValueContainer
		return *((*this->iterator).get());
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
	 * The wrapped std::iterator, from the NyanSet std::unordered_set.
	 */
	iter_type iterator;
};


/**
 * Nyan value to store a unordered set of things.
 *
 * Underlying is a std:unordered_set, which stores NyanValues.
 */
class NyanSet : public NyanContainer {
public:
	using NyanContainer::iterator;
	using NyanContainer::const_iterator;

	using value_storage = std::unordered_set<NyanValueContainer>;
	using value_iterator = value_storage::iterator;
	using value_const_iterator = value_storage::const_iterator;

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


	iterator begin() override;
	iterator end() override;
	const_iterator begin() const override;
	const_iterator end() const override;

	value_storage::iterator values_begin();
	value_storage::iterator values_end();
	value_storage::const_iterator values_begin() const;
	value_storage::const_iterator values_end() const;

protected:
	void apply_value(const NyanValue *value, nyan_op operation) override;
	bool equals(const NyanValue &other) const override;
	const std::unordered_set<nyan_op> &allowed_operations(nyan_type value_type) const override;

	value_storage values;
};


} // namespace nyan

#endif
