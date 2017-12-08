// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <deque>
#include <unordered_map>


namespace nyan::datastructure {


/**
 * Linked list to support removing elements by pointer.
 * `T` must be hashable as a key in `value_storage_t`.
 */
template <typename T>
class OrderedSet {
public:
	OrderedSet() = default;
	~OrderedSet() = default;

	OrderedSet(const OrderedSet &other) {
		for (auto &value : other) {
			this->insert(value);
		}
	}

	const OrderedSet &operator =(const OrderedSet &other) {
		for (auto &value : other) {
			this->insert(value);
		}
	}

	// no moves allowed because they invalidate
	// the ordering iterators.
	OrderedSet(OrderedSet &&other) = delete;
	const OrderedSet &operator =(OrderedSet &&other) = delete;

	/**
	 * Type of value contained in the set.
	 */
	using value_type = T;


	/**
	 * Type of the list that preserves the element order.
	 */
	using order_list_t = std::deque<const T *>;


	/**
	 * Iterator for list elements.
	 */
	using list_iter = typename order_list_t::iterator;


	/**
	 * Type of the value set.
	 * Stores to the ordered list iterator so we can access the order.
	 */
	using value_storage_t = std::unordered_map<T, list_iter>;

protected:
	/**
	 * OrderedSet iterator.
	 *
	 * Basically relays to the list iterator, but it returns
	 * a T& because of double-dereferencing the iterator.
	 * That way, you can iterate over the actual set contents
	 * in the right order.
	 *
	 * Thanks C++ for such a small and readable implementation.
	 */
	template<typename elem_type>
	class OrderedSetIterator
		: public std::iterator<std::forward_iterator_tag, elem_type> {
	public:
		using iter_type = typename std::conditional<
			std::is_const<elem_type>::value,
			typename order_list_t::const_iterator,
			typename order_list_t::iterator>::type;

		using set_type = typename std::conditional<
			std::is_const<elem_type>::value,
			const OrderedSet,
			OrderedSet>::type;

		OrderedSetIterator(set_type &set, bool use_start)
			:
			iter{use_start ?
			     set.value_order.begin()
			     :
			     set.value_order.end()} {}

		virtual ~OrderedSetIterator() = default;

		/**
		 * Advance the inner iterator to the next element.
		 */
		OrderedSetIterator &operator ++() {
			++this->iter;
			return *this;
		}

		/**
		 * Get the element the inner iterator points to.
		 * The first iterator is the order-iterator.
		 * Dereferencing it provides a pointer to the data.
		 * Dereferencing that pointer gets the data reference.
		 */
		elem_type &operator *() const {
			return *(*this->iter);
		}

		/**
		 * Check if this iterator points to the same element
		 * as the other iterator.
		 */
		bool operator ==(const OrderedSetIterator& other) const {
			return (this->iter == other.iter);
		}

		/**
		 * Check if the iterator does not point to the same
		 * element as the other iterator.
		 */
		bool operator !=(const OrderedSetIterator& other) const {
			return not (*this == other);
		}

	protected:
		iter_type iter;
	};


public:
	// just have a const_iterator, because sets don't support
	// changing values in them!
	using const_iterator = OrderedSetIterator<const T>;


protected:
	/**
	 * list to preserve the set order.
	 */
	order_list_t value_order;


	/**
	 * unordered entry storage.
	 */
	value_storage_t values;


public:
	/**
	 * Add an entry to the orderedset.
	 * If already in the set, move entry to the end.
	 */
	bool insert(const T &value) {
		// maybe it is even faster if we check existence with
		// this->values.find(value) first, although then
		// we need to hash value twice: once for the find
		// and once for the insert.
		// most of the time it won't be in the list,
		// so i chose this approach.

		// try new insert, get the iterator to the insertion place
		// as list position, use a dummy which gets replaced below
		auto [value_pos, new_insert] = this->values.emplace(
			value, list_iter{});

		if (not new_insert) {
			// inserted again -> move it to the back in the order list
			// -> delete the current order list entry
			this->value_order.erase(value_pos->second);
		}

		// the pointer is only invalidated when the element is deleted
		// so we can store it in the order list
		// the pointer is const, as the hashmap key must not change
		// => this orderedset can't have modifying iterators!
		const T *value_ptr = &(value_pos->first);

		// add a ptr to the value to the element order list at the end
		auto list_ins = this->value_order.insert(
			std::end(this->value_order), value_ptr
		);

		// and store the list iterator to the map
		value_pos->second = list_ins;
		return new_insert;
	}

	// TODO: add add(T &&value) function


	/**
	 * Remove all entries from the set.
	 */
	void clear() {
		this->values.clear();
		this->value_order.clear();
	}


	/**
	 * Erase an element from the set.
	 */
	size_t erase(const T &value) {
		auto it = this->values.find(value);
		if (it == std::end(this->values)) {
			return 0;
		}

		// remove the order entry
		this->value_order.erase(it->second);

		// and remove the value mapping
		this->values.erase(it);

		return 1;
	}


	/**
	 * Is the specified value stored in this set?
	 */
	bool contains(const T &value) const {
		return (this->values.find(value) != std::end(this->values));
	}


	/**
	 * Return the number of elements stored.
	 */
	size_t size() const {
		return this->value_order.size();
	}


	/** provide the begin iterator of this set */
	const_iterator begin() const {
		return OrderedSetIterator<const T>{*this, true};
	}


	/** provide the end iterator of this set */
	const_iterator end() const {
		return OrderedSetIterator<const T>{*this, false};
	}
};

} // namespace nyan::datastructure
