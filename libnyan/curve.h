// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once

#include <map>

#include "config.h"
#include "error.h"
#include "util.h"


namespace nyan {

template<typename T>
class Curve {
public:

	using container_t = std::map<order_t, T>;

	Curve() {}

	/**
	 * Get the latest value at given time.
	 */
	const T &at(const order_t &time) const {
		// search for element which is greater than time
		auto it = this->container.upper_bound(time);
		if (it == std::begin(this->container)) {
			throw InternalError{"requested time lower than first entry"};
		}

		// go one back, so it's less or equal the requested time.
		--it;
		return it->second;
	}

	/**
	 * Get the value at the exact time.
	 */
	T *at_exact(const order_t &time) {
		auto it = this->container.find(time);
		if (it == std::end(this->container)) {
			return nullptr;
		}

		return &it->second;
	}

	/**
	 * No data is stored in the curve.
	 */
	bool empty() const {
		return this->container.empty();
	}

	/**
	 * Add a new value at the given time.
	 */
	T &insert_drop(const order_t &time, T &&value) {
		auto it = this->container.upper_bound(time);

		// remove all elements greater than the requested time
		for (; it != std::end(this->container); ++it) {
			this->container.erase(it);
		}

		// insert the new keyframe
		auto ret = this->container.insert({time, std::move(value)});
		if (unlikely(ret.second == false)) {
			throw InternalError{"did not insert value, it existed before"};
		}

		return ret.first->second;
	}

protected:
	container_t container;
};

} // namespace nyan
