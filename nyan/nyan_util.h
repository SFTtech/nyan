// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_UTIL_H_
#define NYAN_UTIL_H_

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "nyan_error.h"

/*
 * Branch prediction tuning.
 * The expression is expected to be true (=likely) or false (=unlikely).
 */
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

namespace nyan {
namespace util {
/**
 * Read a file from the filesystem and return the contents.
 */
std::string read_file(const std::string &filename);


/**
 * Just like python's "delim".join(container)
 * Fak u C++.
 */
template <typename T>
std::string strjoin(const std::string &delim,
                    const std::vector<T> &container) {

	std::ostringstream builder;

	size_t cnt = 0;
	for (auto &entry : container) {
		if (cnt > 0) {
			builder << delim;
		}
		builder << entry;
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
		idx{0} {}

	virtual ~Iterator() = default;

	const containerT &container;
	size_t idx;

	const T *next() {
		if (not this->full()) {
			throw ParserError("reached end of file");
		}
		return &this->container[this->idx++];
	}
	const T *preview() {
		if (not this->full()) {
			throw ParserError("can't look ahead: reached end of file");
		}
		return &this->container[this->idx];
	}
	bool full() const { return this->idx < this->container.size(); }
	bool empty() const { return not this->full(); }
};

} // namespace util
} // namespace nyan

#endif
