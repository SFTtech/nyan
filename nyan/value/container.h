// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <iterator>

#include "value.h"


namespace nyan {


/**
 * Base class for container iterators.
 * Inherited from and implemented for each nyan container
 * in order to support iteration.
 *
 * The child class adds stuff so it can iterate over whatever the
 * target container needs for iteration state storage.
 *
 * The begin() and end() functions of the container class
 * instanciate this by wrapping it in the ContainerIterator below.
 */
template<typename elem_type>
class ContainerIterBase : public std::iterator<std::forward_iterator_tag,
                                               elem_type> {
public:
	using this_type = ContainerIterBase<elem_type>;

	ContainerIterBase() = default;
	virtual ~ContainerIterBase() = default;

	/**
	 * Advance the iterator to the next element.
	 */
	virtual this_type &operator ++() = 0;

	/**
	 * Get the element the iterator is currently pointing to.
	 */
	virtual elem_type &operator *() const = 0;

	/**
	 * Compare if both iterators are pointing
	 * to the same container position.
	 */
	bool operator ==(const ContainerIterBase &other) const {
		return (typeid(*this) == typeid(other)) and this->equals(other);
	}

protected:
	/**
	 * Actually perform the comparison if both iterators
	 * point to the same element.
	 */
	virtual bool equals(const ContainerIterBase &other) const = 0;
};


/**
 * Nyan container iterator wrapper class.
 * Wraps the ContainerIterBase so we can have virtual calls.
 *
 * Just relays the calls to the wrapped actual container.
 */
template<typename T>
class ContainerIterator : public std::iterator<std::forward_iterator_tag, T> {
public:
	using elem_type = T;
	using real_iterator = ContainerIterBase<elem_type>;


	ContainerIterator() = default;
	ContainerIterator(std::unique_ptr<ContainerIterBase<elem_type>> &&real) noexcept
		:
		iter{std::move(real)} {}

	ContainerIterator(const ContainerIterator &other)
		:
		iter{std::make_unique(other)} {}

	ContainerIterator(ContainerIterator &&other) noexcept
		:
		iter{std::move(other.iter)} {}

	ContainerIterator &operator =(const ContainerIterator &other) {
		this->iter = std::make_unique(other);
	}

	ContainerIterator &operator =(ContainerIterator &&other) noexcept {
		this->iter = std::move(other);
	}

	virtual ~ContainerIterator() = default;

	/**
	 * Advance the inner iterator to the next element.
	 */
	ContainerIterator &operator ++() {
		++(*this->iter);
		return *this;
	}

	/**
	 * Get the element the inner iterator points to.
	 */
	elem_type &operator *() const {
		return *(*this->iter);
	}

	/**
	 * Check if this iterator points to the same container element
	 * as the other iterator.
	 */
	bool operator ==(const ContainerIterator& other) const {
		return (this->iter == other.iter) or (*this->iter == *other.iter);
	}

	/**
	 * Check if the iterator does not point to the same container element
	 * as the other iterator.
	 */
	bool operator !=(const ContainerIterator& other) const {
		return not (*this == other);
	}

protected:
	/**
	 * The real iterator.
	 * Just wrapped here to enable virtual function calls.
	 */
	std::unique_ptr<ContainerIterBase<elem_type>> iter;
};


/**
 * Implementation for wrapping standard STL container forward iterators.
 */
template <typename iter_type, typename elem_type>
class DefaultIterator : public ContainerIterBase<elem_type> {
public:
	using this_type = DefaultIterator<iter_type, elem_type>;
	using base_type = ContainerIterBase<elem_type>;

	explicit DefaultIterator(iter_type &&iter)
		:
		iterator{std::move(iter)} {}

	/**
	 * Advance the iterator to the next element in the set.
	 */
	base_type &operator ++() override {
		++this->iterator;
		return *this;
	}

	/**
	 * Return the iterator value.
	 */
	elem_type &operator *() const override {
		return *this->iterator;
	}

protected:
	/**
	 * Compare two iterators for pointing at the same element.
	 */
	bool equals(const base_type &other) const override {
		auto other_me = dynamic_cast<const this_type &>(other);
		return (this->iterator == other_me.iterator);
	}

	/**
	 * The wrapped std::iterator.
	 */
	iter_type iterator;
};


/**
 * Value that can store other Values.
 * Provides iterators and add/remove methods.
 */
class Container : public Value {
public:
	using iterator = ContainerIterator<Value>;
	using const_iterator = ContainerIterator<const Value>;

	using holder_iterator = ContainerIterator<ValueHolder>;
	using holder_const_iterator = ContainerIterator<const ValueHolder>;

	Container();
	virtual ~Container() = default;

	/**
	 * Return the number of elements in this container.
	 */
	virtual size_t size() const = 0;

	/**
	 * Add the given value to this container.
	 * @returns if the value was added successfully,
	 * false if it was already in there.
	 */
	virtual bool add(const ValueHolder &value) = 0;

	/**
	 * Test if this value is in the container.
	 */
	virtual bool contains(const ValueHolder &value) const = 0;

	/**
	 * Remove the given value from the container if it is in there.
	 * @returns if if was removed successfully.
	 */
	virtual bool remove(const ValueHolder &value) = 0;

	/**
	 * Get an iterator to the first element in that container.
	 */
	virtual iterator begin() = 0;

	/**
	 * Get an iterator to the slot beyond the last element in the container.
	 */
	virtual iterator end() = 0;

	/**
	 * Get a constant iterator to the first element in that container.
	 */
	virtual const_iterator begin() const = 0;

	/**
	 * Get a constant iterator to the slot beyond the last element in the
	 * container.
	 */
	virtual const_iterator end() const = 0;

	/**
	 * Guarantee a const_iterator beginning.
	 */
	const_iterator cbegin() const { return this->begin(); };

	/**
	 * Guarantee a const_iterator end.
	 */
	const_iterator cend() const { return this->end(); };

	/**
	 * Get an iterator to the first value holder in that container.
	 */
	virtual holder_iterator values_begin() = 0;

	/**
	 * Get an iterator to the slot beyond the last value holder
	 * in the container.
	 */
	virtual holder_iterator values_end() = 0;

	/**
	 * Get a constant iterator to the first value holder in the container.
	 */
	virtual holder_const_iterator values_begin() const = 0;

	/**
	 * Get a constant iterator to the slot beyond the last value holder
	 * in the container.
	 */
	virtual holder_const_iterator values_end() const = 0;

	/**
	 * Guarantee a const_iterator to the value iterator beginning.
	 */
	holder_const_iterator values_cbegin() const { return this->values_begin(); };

	/**
	 * Guarantee a const_iterator to the value iterator end.
	 */
	holder_const_iterator values_cend() const { return this->values_end(); };
};

} // namespace nyan
