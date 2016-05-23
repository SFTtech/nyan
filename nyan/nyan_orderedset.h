// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_ORDEREDSET_H_
#define NYAN_ORDEREDSET_H_

#include <list>
#include <unordered_set>

#include "nyan_error.h"
#include "nyan_ptr_container.h"


namespace nyan {

/**
 * Linked list to support removing elements by pointer.
 * T must be something that is normally wrapped in NyanPtrContainer.
 */
template <typename T>
class OrderedSet {
public:
	OrderedSet() {}
	virtual ~OrderedSet() {}

protected:
	// magic forward declaration
	struct set_entry;

	/**
	 * Type of the list that preserves the element order.
	 */
	using order_list_t = std::list<const OrderedSet::set_entry *>;

	/**
	 * list to preserve the set order.
	 */
	order_list_t value_order;

	/**
	 * Iterator for list elements.
	 */
	using list_iter = typename order_list_t::iterator;

	/**
	 * Hashmap entry to find the list index.
	 */
	struct set_entry {
		set_entry(T &&val) : value{std::move(val)} {}
		set_entry(T *val) : value{val} {}
		set_entry(set_entry &&other)
			:
			value{std::move(other.value)},
			list_iter{other.list_iter} {}

		const T &operator ->() const { return this->value; }

		const T &get() const { return this->value; }

		struct hash {
			size_t operator ()(const set_entry &entry) const {
				return std::hash<T>{}(entry.value);
			}
		};

		bool operator ==(const set_entry &other) const {
			return this->value == other.value;
		}

		bool operator !=(const set_entry &other) const {
			return this->value != other.value;
		}

		void set_list_iter(const list_iter &it) {
			this->list_iter = it;
		}

		/**
		 * Value of this entry.
		 */
		T value;

		/**
		 * Associated list element.
		 */
		list_iter list_iter;
	};


	/**
	 * Type of the value set.
	 */
	using value_set_t = std::unordered_set<set_entry, typename set_entry::hash>;

	/**
	 * Element iterators type.
	 */
	using iterator = typename order_list_t::iterator;


	/**
	 * Element const iterator type.
	 */
	using const_iterator = typename order_list_t::const_iterator;


	/**
	 * unordered entry storage.
	 */
	value_set_t values;


public:
	/**
	 * Add an entry to the orderedset.
	 * If already in the set, move entry to the end.
	 */
	bool add(T &&value) {
		auto inserted = this->values.insert(std::move(value));

		// element where the ordered list will point to.
		// can be the already known set entry.
		auto insert_pos = std::get<0>(inserted);

		// true when the element is _not_ known and needs to be inserted
		bool new_insert = std::get<1>(inserted);

		// remove the old list entry to move it to the end then
		if (not new_insert) {
			this->value_order.erase(insert_pos->list_iter);
		}

		// add it to the element order list at the end
		auto list_ins = this->value_order.insert(
			std::end(this->value_order), &(*insert_pos)
		);

		// in the set, remember where the element is in the list
		// holy fuck is this dirty.
		// somehow, the "insert_pos" is const, while the specification
		// of unordered_set.insert() says it's not.
		// so we need to get rid of the const in order to set
		// the list position in the set element.
		const_cast<set_entry *>(&(*insert_pos))->set_list_iter(list_ins);

		return new_insert;
	}

	bool contains(T &&value) const {
		throw NyanInternalError{"TODO orderedset contains"};
	}

	iterator begin() noexcept {
		return this->value_order.begin();
	}

	const_iterator begin() const noexcept {
		return this->value_order.begin();
	}

	iterator end() noexcept {
		return this->value_order.end();
	}

	const_iterator end() const noexcept {
		return this->value_order.end();
	}
};

} // namespace nyan

#endif
