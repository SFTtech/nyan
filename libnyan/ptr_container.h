// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_PTR_CONTAINER_H_
#define NYAN_NYAN_PTR_CONTAINER_H_

#include <memory>


namespace nyan {

/**
 * Container class to contain some data, either by
 * owning one or by pointing to an existing one.
 */
template<typename T>
class PtrContainer {
public:
	PtrContainer()
		:
		is_ptr{true},
		data_owned{nullptr},
		data_ptr{nullptr} {}

	/**
	 * Type of this container.
	 */
	using this_type = PtrContainer<T>;

	/**
	 * Entry data type.
	 */
	using entry_type = T *;

	/**
	 * Create a non-owning container from a raw pointer.
	 */
	PtrContainer(T *val)
		:
		is_ptr{true},
		data_owned{nullptr},
		data_ptr{val} {}

	/**
	 * Create an owning container from some unique ptr.
	 */
	PtrContainer(std::unique_ptr<T> &&val) noexcept
		:
		is_ptr{false},
		data_owned{std::move(val)},
		data_ptr{nullptr} {}

	/**
	 * Replace this container by another container.
	 */
	PtrContainer(PtrContainer &&other) noexcept
		:
		is_ptr{other.is_ptr},
		data_owned{std::move(other.data_owned)},
		data_ptr{other.data_ptr} {}

	/**
	 * Move assignment from another container.
	 */
	PtrContainer &operator =(PtrContainer &&other) noexcept {
		this->is_ptr = other.is_ptr;
		this->data_owned = std::move(other.data_owned);
		this->data_ptr = other.data_ptr;
		return *this;
	}

	// no copies
	PtrContainer(const PtrContainer &other) = delete;
	PtrContainer &operator =(const PtrContainer &other) = delete;

	virtual ~PtrContainer() = default;


	/**
	 * Set the value to an owned unique_ptr.
	 */
	void set(std::unique_ptr<T> &&val) {
		this->is_ptr = false;
		this->data_owned = std::move(val);
		this->data_ptr = nullptr;
	}

	/**
	 * Set the value to a non-owning pointer.
	 */
	void set(T *val) {
		this->is_ptr = true;
		this->data_owned = nullptr;
		this->data_ptr = val;
	}


	/**
	 * Return a pointer to the contained data.
	 */
	T *get() const noexcept {
		if (this->is_ptr) {
			return this->data_ptr;
		} else {
			return this->data_owned.get();
		}
	}


	/**
	 * Return another ptr container which contains a ptr to the
	 * data stored in this container.
	 */
	this_type get_ref() const noexcept {
		return this_type{this->get()};
	};


	/**
	 * Provide a reference to the data.
	 */
	T &operator *() const noexcept {
		return *this->get();
	}


	/**
	 * Dereference the data for member access.
	 */
	T *operator ->() const noexcept {
		return this->get();
	}


	/**
	 * Return true if this container has any data stored.
	 */
	bool has_data() const {
		return (this->get() != nullptr);
	}


	/**
	 * Return true if this container owns the value
	 * and does not only store a ptr to it.
	 */
	bool is_owning() const {
		return not this->is_ptr;
	}


	/**
	 * Compare the contained values.
	 */
	bool operator ==(const T &other) const {
		return *this->get() == other;
	}


	/**
	 * Compare a container with a value.
	 */
	bool operator ==(const PtrContainer<T> &other) const {
		return *this->get() == *other.get();
	}


	/**
	 * Check if contained values are not equal.
	 */
	bool operator !=(const this_type &other) const {
		return not (*this->get() == *other.get());
	}

protected:
	bool is_ptr;
	std::unique_ptr<T> data_owned;
	T *data_ptr;
};


} // namespace nyan

#endif
