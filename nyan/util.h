// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <algorithm>
#include <concepts>
#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

#include "error.h"


namespace nyan::util {

/**
 * Read a file from the filesystem and return the contents.
 *
 * @param filename Name of the file.
 * @param binary If true, open the file in binary mode, which
 *     will leave newlines untouched.
 *
 * @return String with the file content.
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
 * Returns the string representation of the given type.
 */
template <typename T>
std::string typestring() {
	return demangle(typeid(T).name());
}

/**
 * Returns the string representation of type of the given pointer.
 */
template <typename T>
std::string typestring(const T *ptr) {
	return demangle(typeid(*ptr).name());
}


/**
 * Check if something supports output stream operators.
 */
template <typename T>
concept Streamable = requires(std::ostream &os, T value) {
	{ os << value } -> std::convertible_to<std::ostream &>;
};


/**
 * Check if something is a container and provides value type correctly.
 */
template <typename T>
concept Container = requires(T thing) {
	// check for value_type type-member and iterator access.
	{*std::begin(thing)} -> std::convertible_to<typename T::value_type>;
	std::begin(thing) == std::end(thing);
};


/**
 * Helper function to convert something to a string.
 * Used as default function in strjoin.
 */
template <typename T>
const std::string &convert_str(const T &in) {
	return in;
}

/**
 * Helper function to stream something to a ostringstream.
 */
template <Streamable T>
void stream_container_elem(std::ostringstream &builder, const T &in) {
	builder << in;
}


/**
 * Join a container with a delimiter to a given stream.
 * Except the stream-thing, it's like python's str.join()
 *
 * @param builder Stringstream to feed the data in.
 * @param delim Delimiter to add in between the elements.
 * @param container The container with the elements to join together.
 * @param func Function to fetch the string for a container item.
 *
 * @return reference to the given builder for chaining.
 */
template <Container T>
std::ostringstream &
strjoin(
	std::ostringstream &builder,
	const std::string &delim,
	const T &container,
	const std::function<void(std::ostringstream &, const typename T::value_type &)>
	func=&stream_container_elem<typename T::value_type>
) {

	bool delim_active = false;
	for (auto &entry : container) {
		if (delim_active) {
			builder << delim;
		}

		func(builder, entry);
		delim_active = true;
	}

	return builder;
}


/**
 * Like python's "delim".join(iterable).
 * func is a function of how to stream each element of the container
 *
 * @param delim Delimeter placed between the joined strings.
 * @param container Iterable container holding the strings.
 * @param func Function for retrieving a string from a container item.
 *
 * @return String containing the joined strings.
 */
template <Container T>
std::string strjoin(
	const std::string &delim,
	const T &container,
	const std::function<const std::string_view(const typename T::value_type &)>
	func=&convert_str<typename T::value_type>
) {
	std::ostringstream builder;
	strjoin(
		builder, delim, container,
		[&func](std::ostringstream &stream,
		        const typename T::value_type &elem) {
			auto &&elem_str = func(elem);
			stream << elem_str;
		}
	);
	return std::move(builder).str();
}


/**
 * Split a string at a delimiter, push the result back in an iterator.
 * Why doesn't the fucking standard library have std::string::split(delimiter)?
 *
 * @tparam ret_t Return type.
 *
 * @param[in]  txt String that is split.
 * @param[in]  delimiter Delimiter char at which the string is split.
 * @param[out] result Splitted string with type \p ret_t.
 */
template<typename ret_t>
void split(const std::string &txt, char delimiter, ret_t result) {
	std::stringstream splitter;
	splitter.str(txt);
	std::string part;

	while (std::getline(splitter, part, delimiter)) {
		*result = part;
		result++;
	}
}


/**
 * Split a string at a delimiter into a vector.
 * Internally, uses the iterator splitter.
 *
 * @param txt String that is split.
 * @param delim Delimiter char at which the string is split.
 *
 * @return List of string components from the splitting.
 */
std::vector<std::string> split(const std::string &txt, char delim);


/**
 * Check if a string ends with another given string.
 *
 * @param txt String that is checked.
 * @param end String that is compared with the end of \p txt.
 *
 * @return true if \p txt ends with \p end, else false.
 */
bool ends_with(const std::string &txt, const std::string &end);


/**
 * Extend a vector with elements, without destroying the
 * source of elements.
 *
 * @tparam T Element type.
 *
 * @param vec Vector that is extended.
 * @param ext Vector used as a source of elements.
 */
template <typename T>
void vector_extend(std::vector<T> &vec, const std::vector<T> &ext) {
	vec.reserve(vec.size() + ext.size());
	vec.insert(std::end(vec), std::begin(ext), std::end(ext));
}


/**
 * Extend a vector with elements using move semantics.
 *
 * @tparam T Element type.
 *
 * @param vec Vector that is extended.
 * @param ext Vector used as a source of elements.
 */
template <typename T>
void vector_extend(std::vector<T> &vec, std::vector<T> &&ext) {
	if (vec.empty()) {
		vec = std::move(ext);
	}
	else {
		vec.reserve(vec.size() + ext.size());
		std::move(std::begin(ext), std::end(ext), std::back_inserter(vec));
		ext.clear();
	}
}


/**
 * Membership check for some container.
 *
 * @tparam T Container type.
 * @tparam V Type of checked value.
 *
 * @param container Container that is searched.
 * @param value Value that is searched for.
 *
 * @return true if the value is in the container, else false.
 */
template <typename T, typename V>
bool contains(const T &container, const V &value) {
	if (std::find(std::begin(container),
	              std::end(container),
	              value) == std::end(container)) {
		return false;
	}
	else {
		return true;
	}
}


/**
 * Creates a hash value as a combination of two other hashes. Can be called incrementally to create
 * hash value from several variables.
 *
 * @param hash1 First hash.
 * @param hash2 Second hash.
 */
size_t hash_combine(size_t hash1, size_t hash2);


/**
 * Helper function to be used as failure case with constexpr-ifs:
 *
 * if constexpr (bla) {
 *     ...
 * }
 * else {
 *     match_failure();
 * }
 */
template <bool flag = false>
void match_failure() {
	static_assert(flag, "no static branch match found");
}

/**
 * Test if the given ptr is an instance of a base
 */
template <typename Base, typename T>
inline bool isinstance(const T *ptr) {
	return dynamic_cast<const Base *>(ptr) != nullptr;
}

/**
 * Test if the given reference is an instance of some base.
 */
template <typename Base, typename T>
requires (!std::is_pointer_v<T>)
inline bool isinstance(const T &ref) {
	return dynamic_cast<const Base *>(&ref) != nullptr;
}

} // namespace nyan::util
