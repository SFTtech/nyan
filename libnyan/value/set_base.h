// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <unordered_set>
#include <vector>

#include "../container.h"
#include "../ptr_container.h"
#include "value.h"
#include "container.h"


namespace nyan {

class Set;

/**
 * Container iterator for the Set.
 * Determines the iterator type by looking at elem_type,
 * then fetches the type from set_type::iterator or const_iterator.
 */
template<typename elem_type, typename set_type>
class SetIterator : public ContainerIterBase<elem_type> {
public:

	using this_type = SetIterator<elem_type, set_type>;
	using base_type = ContainerIterBase<elem_type>;
	using iter_type = typename set_type::value_const_iterator;

	SetIterator(set_type *set, bool at_start)
		:
		iterator{at_start ? set->values_begin() : set->values_end()} {}

	virtual ~SetIterator() = default;

	/**
	 * Advance the iterator to the next element in the set.
	 */
	base_type &operator ++() override {
		++this->iterator;
		return *this;
	}

	/**
	 * Get the element the iterator is currently pointing to.
	 */
	elem_type &operator *() const override {
		// unpack the ValueContainer
		auto &&it = *this->iterator;
		return *(it.get());
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
 * Nyan value to store a unordered set of things.
 *
 * T is the underlying storage type to store the Values.
 */
template <typename T>
class SetBase : public Container {
public:
	using Container::iterator;
	using Container::const_iterator;

	using value_storage = T;
	using value_const_iterator = typename value_storage::const_iterator;

	SetBase() = default;
	virtual ~SetBase() = default;


	size_t hash() const override {
		throw Error{"Sets are not hashable."};
	}

	size_t size() const override {
		return this->values.size();
	}


	iterator begin() override {
		throw Error{"Sets are not non-const-iterable. make it const by using e.g. for (auto &it = util::as_const(container))"};
	}


	iterator end() override {
		return this->begin();
	}


	const_iterator begin() const override {
		// uuuh yes. this creates an iterator to the contained elements.
		// the iterator itself is a stack object, which relays the calls
		// to the actual iterator.
		//
		// does semi-magic:
		// We create a heap-allocated iterator and then wrap it
		// to do the virtual calls.
		// It is designed to be callable by a generic interface
		// that all Containers support.
		//
		// iterator::elem_type = the single element type of the iteration.
		// Set             = the target set class,
		//                       which is non-const in this begin()
		//                       implementation,
		//                       but not in the begin() below.
		// (this, true)        = use this set as target, use the beginning.
		auto real_iterator = std::make_unique<
			SetIterator<const_iterator::elem_type,
			                const SetBase>>(this, true);

		return const_iterator{std::move(real_iterator)};
	}


	const_iterator end() const override {
		// see explanation in the begin() above
		auto real_iterator = std::make_unique<
			SetIterator<const_iterator::elem_type,
			                const SetBase>>(this, false);

		return const_iterator{std::move(real_iterator)};
	}

	typename value_storage::const_iterator values_begin() const {
		return this->values.begin();
	}

	typename value_storage::const_iterator values_end() const {
		return this->values.end();
	}

protected:
	/**
	 * test if the same values are in those sets
	 */
	bool equals(const Value &other) const override {
		auto &other_val = dynamic_cast<const SetBase &>(other);
		throw InternalError{"TODO set equality"};
	}

	value_storage values;
};

} // namespace nyan
