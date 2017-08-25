// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once

#include <functional>
#include <map>

#include "config.h"
#include "error.h"
#include "util.h"


namespace nyan {

template<typename T>
class Curve {
public:

	using container_t = std::map<order_t, T>;

	using fallback_t = std::function<const T &(const order_t)>;


	Curve() {}

#ifdef CURVE_FALLBACK_FUNCTION
	Curve(const fallback_t &func)
		:
		fallback{func} {}

	/**
	 * Fallback function, used if a time before the first
	 * entry is used.
	 */
	fallback_t fallback;
#endif

	// TODO: maybe add variants of the functions below
	//       which also return the time the keyframe belongs to.

	/**
	 * Get the latest value at given time.
	 */
	const T &at(const order_t time) const {
		// search for element which is greater than time
		auto it = this->container.upper_bound(time);
		if (it == std::begin(this->container)) {
#ifdef CURVE_FALLBACK_FUNCTION
			if (likely(this->fallback)) {
				return this->fallback(time);
			}
			else {
#endif
				throw InternalError{
					"requested time lower than first curve entry"
				};
#ifdef CURVE_FALLBACK_FUNCTION
			}
#endif
		}

		// go one back, so it's less or equal the requested time.
		--it;
		return it->second;
	}

	/**
	 * Like `at`, but returns nullptr if no keyframe was found.
	 */
	const T *at_find(const order_t time) const {
		auto it = this->container.upper_bound(time);
		if (it == std::begin(this->container)) {
			return nullptr;
		}
		--it;
		return &it->second;
	}

	/**
	 * Get the value at the exact time.
	 */
	const T *at_exact(const order_t time) const {
		auto it = this->container.find(time);
		if (it == std::end(this->container)) {
			return nullptr;
		}

		return &it->second;
	}

	/**
	 * Get the value which active earlier than given time.
	 */
	const T &before(const order_t time) const {
		// search for element which is not less than the given time.
		auto it = this->container.lower_bound(time);
		if (it == std::begin(this->container)) {
			throw InternalError{"curve has no previous keyframe"};
		}

		// go one back, so it's less than the requested time.
		--it;
		return it->second;
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
	T &insert_drop(const order_t time, T &&value) {
		auto it = this->container.lower_bound(time);

		// remove all elements greater or equal the requested time
		this->container.erase(it, std::end(this->container));

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
