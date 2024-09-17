// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <unordered_set>
#include <vector>

#include "../api_error.h"
#include "../compiler.h"
#include "../util.h"
#include "container.h"


namespace nyan {

class Set;

/**
 * Container iterator for the Set.
 *
 * Used for walking over the contained values,
 * i.e. it unpacks the ValueHolders!
 *
 */
template <typename iter_type, typename elem_type>
class SetIterator : public ContainerIterBase<elem_type> {
public:
	using this_type = SetIterator<iter_type, elem_type>;
	using base_type = ContainerIterBase<elem_type>;

	explicit SetIterator(iter_type &&iter) :
		iterator{std::move(iter)} {}

	/**
	 * Advance the iterator to the next element in the set.
	 */
	base_type &operator++() override {
		++this->iterator;
		return *this;
	}

	/**
	 * Get the element the iterator is currently pointing to.
	 */
	elem_type &operator*() const override {
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
 * Nyan value to store set of things.
 *
 * T is the underlying storage type to store the Values.
 */
template <typename T>
class SetBase : public Container {
public:
	using Container::const_iterator;
	using Container::holder_const_iterator;
	using Container::holder_iterator;
	using Container::iterator;

	using value_storage = T;
	using element_type = typename value_storage::value_type;
	using value_const_iterator = typename value_storage::const_iterator;


	SetBase() = default;
	virtual ~SetBase() = default;


	size_t hash() const override {
		throw APIError{"Sets are not hashable."};
	}


	size_t size() const override {
		return this->values.size();
	}


	void clear() {
		this->values.clear();
	}


	const value_storage &get() const {
		return this->values;
	}


	iterator begin() override {
		throw APIError{
			"Sets are only const-iterable. "
			"make it const by using e.g. "
			"for (auto &it = std::as_const(container))"};
	}

	iterator end() override {
		// also throw the error above.
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
		// Set                 = the target set class,
		//                       which is non-const in this begin()
		//                       implementation,
		//                       but not in the begin() below.
		// (this, true)        = use this set as target, use the beginning.
		auto real_iterator = std::make_unique<
			SetIterator<value_const_iterator,
		                const_iterator::elem_type>>(std::begin(this->values));

		return const_iterator{std::move(real_iterator)};
	}


	const_iterator end() const override {
		// see explanation in the begin() above
		auto real_iterator = std::make_unique<
			SetIterator<value_const_iterator,
		                const_iterator::elem_type>>(std::end(this->values));

		return const_iterator{std::move(real_iterator)};
	}


	holder_iterator values_begin() override {
		throw APIError{
			"Set values holders are not non-const-iterable."};
	}


	holder_iterator values_end() override {
		// also throw the error above.
		return this->values_begin();
	}


	/**
	 * Get an iterator to the underlying set storage.
	 * Contrary to the above, this will allow to get the
	 * ValueHolders.
	 */
	holder_const_iterator values_begin() const override {
		auto real_iterator = std::make_unique<
			DefaultIterator<value_const_iterator,
		                    holder_const_iterator::elem_type>>(
			std::begin(this->values));

		return holder_const_iterator{std::move(real_iterator)};
	}

	/**
	 * Iterator to end of the underlying storage.
	 */
	holder_const_iterator values_end() const override {
		auto real_iterator = std::make_unique<
			DefaultIterator<value_const_iterator,
		                    holder_const_iterator::elem_type>>(
			std::end(this->values));

		return holder_const_iterator{std::move(real_iterator)};
	}

protected:
	/**
	 * Update this set with another set with the given operation.
	 */
	bool apply_value(const Value &value, nyan_op operation) override {
		const Container *change = dynamic_cast<const Container *>(&value);

		if (unlikely(change == nullptr)) {
			using namespace std::string_literals;
			throw InternalError{
				"set value application was not a container, it was: "s
				+ util::demangle(typeid(value).name())
				+ " and couldn't cast to "
				+ util::demangle(typeid(change).name())};
		}

		switch (operation) {
		case nyan_op::ASSIGN:
			this->values.clear();
			// fall through

		case nyan_op::UNION_ASSIGN:
		case nyan_op::ADD_ASSIGN: {
			auto it = change->values_begin();
			for (auto end = change->values_end(); it != end; ++it) {
				this->values.insert(*it);
			}
			break;
		}

		case nyan_op::SUBTRACT_ASSIGN: {
			auto it = change->values_begin();
			for (auto end = change->values_end(); it != end; ++it) {
				this->values.erase(*it);
			}
			break;
		}

		case nyan_op::INTERSECT_ASSIGN: {
			// only keep the values that are in both.

			std::vector<element_type> keep;
			keep.reserve(this->values.size());

			// as an intersection with a set is allowed,
			// we have to walk over the ordered set
			// instead of the set value to keep the order.
			auto it = this->values_begin();
			for (auto end = this->values_end(); it != end; ++it) {
				if (change->contains(*it)) {
					keep.push_back(*it);
				}
			}

			this->values.clear();

			for (auto &value : keep) {
				this->values.insert(value);
			}

			break;
		}

		default:
			throw InternalError{"unknown set value application"};
		}

		return true;
	}


	/**
	 * test if the same values are in those sets
	 */
	bool equals(const Value &other) const override {
		auto &other_val = dynamic_cast<const SetBase &>(other);

		// TODO: this only compares for set values,
		//       but for the orderedset, the order might matter!

		if (this->size() != other_val.size()) {
			return false;
		}

		auto it = this->values_begin();
		for (auto end = this->values_end(); it != end; ++it) {
			if (not other_val.contains(*it)) {
				return false;
			}
		}

		return true;
	}

	/**
	 * Set value storage.
	 * Type is determined by the set specialization.
	 */
	value_storage values;
};

} // namespace nyan
