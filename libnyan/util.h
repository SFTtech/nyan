// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_UTIL_H_
#define NYAN_UTIL_H_

#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "error.h"

/*
 * Branch prediction tuning.
 * The expression is expected to be true (=likely) or false (=unlikely).
 */
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

namespace nyan {
namespace util {

/**
 * C++17 provides std::as_const, which does exactly the same:
 * Constify a value by prepending const.
 */
template<typename T> constexpr const T &as_const(T &t) noexcept {
	return t;
}

/**
 * Determine the size of a file.
 */
size_t file_size(const std::string &filename);

/**
 * Read a file from the filesystem and return the contents.
 * Optionally, open it in binary mode, which will leave newlines untouched.
 */
std::string read_file(const std::string &filename, bool binary=false);


/**
 * Demangles a symbol name.
 *
 * On failure, the mangled symbol name is returned.
 */
std::string demangle(const char *symbol);

/**
 * Return the demangled symbol name for a given code address.
 */
std::string symbol_name(const void *addr, bool require_exact_addr=true, bool no_pure_addrs=false);


/**
 * Just like python's "delim".join(container)
 * func is a function to extract the string
 * from each element of the container.
 * Fak u C++.
 */
template <typename T>
std::string strjoin(const std::string &delim,
                    const std::vector<T> &container,
                    const std::function<const std::string(const T &)> func=[](const T &in) {return in;}) {

	std::ostringstream builder;

	size_t cnt = 0;
	for (auto &entry : container) {
		if (cnt > 0) {
			builder << delim;
		}

		builder << func(entry);
		cnt += 1;
	}

	return builder.str();
}


/**
 * Python-yield like iterator for containers.
 * You can fetch the next value until nothing is left.
 */
template <typename T, typename containerT=std::vector<T>>
class Iterator {
public:
	Iterator(const containerT &container)
		:
		container{container},
		iter{std::begin(container)},
		reinserted{nullptr} {}

	virtual ~Iterator() = default;

	const containerT &container;
	const std::string source;
	typename containerT::const_iterator iter;
	const T *reinserted;

	const T *next() {
		if (this->reinserted != nullptr) {
			const T *ret = this->reinserted;
			this->reinserted = nullptr;
			return ret;
		}

		if (not this->full()) {
			throw Error{"requested item from empty list"};
		}

		auto ret = &(*this->iter);
		this->iter = std::next(this->iter);
		return ret;
	}

	bool full() const {
		return this->iter != std::end(this->container);
	}

	bool empty() const {
		return not this->full();
	}

	void reinsert(const T *item) {
		this->reinserted = item;
	}
};

} // namespace util
} // namespace nyan

#endif
