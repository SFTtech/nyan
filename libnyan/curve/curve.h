// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once

#include "keyframe_container.h"

namespace nyan {
namespace curve {

template<typename T>
class Curve {
public:
	Curve()
		:
		last_element{this->container.begin()} {}

	/**
	 * Get the latest value at given time.
	 */
	const T &at(const order_t &t) const;

	const T &operator ()(const order_t &now) const {
		return this->get(now);
	}

	bool needs_update(const order_t &at);

	std::pair<order_t, const T &> frame(const order_t &) const;
	std::pair<order_t, const T &> next_frame(const order_t &) const;

	// Inserter mode
	void set_drop(const order_t &at, const T &value);
	void set_insert(const order_t &at, const T &value);

protected:
	KeyframeContainer<T> container;
	mutable typename KeyframeContainer<T>::KeyframeIterator last_element;
};


template <typename T>
const T &Curve<T>::at(const order_t &time) const {
	auto e = this->container.last(time, this->last_element);
	this->last_element = e;
	return e->value;
}


template <typename T>
void Curve<T>::set_drop(const order_t &at, const T &value) {
	auto hint = this->container.erase_after(
		this->container.last(at, this->last_element)
	);
	this->container.insert(at, value, hint);
	this->last_element = hint;
}


template <typename T>
void Curve<T>::set_insert(const order_t &at, const T &value) {
	this->container.insert(at, value, this->last_element);
}


template <typename T>
std::pair<order_t, const T&> Curve<T>::frame(const order_t &time) const {
	auto e = this->container.last(time, this->container.end());
	return std::make_pair(e->time, e->value);
}


template <typename T>
std::pair<order_t, const T&> Curve<T>::next_frame(const order_t &time) const {
	auto e = this->container.last(time, this->container.end());
	e++;
	return std::make_pair(e->time, e->value);
}


template <typename T>
bool Curve<T>::needs_update(const order_t &at) {
	auto e = this->container.last(at, this->container.end());
	if (e->time > at or ++e == this->container.end() or e->time > at) {
		return true;
	} else {
		return false;
	}
}

}} // nyan::curve
