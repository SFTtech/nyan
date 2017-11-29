// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <vector>

#include "config.h"


namespace nyan {

class IDToken;


class Namespace {
	friend struct std::hash<Namespace>;

public:
	explicit Namespace(const fqon_t &token);
	explicit Namespace(const IDToken &token);
	Namespace(const Namespace &other, const std::string &addend);

	virtual ~Namespace() = default;

	void pop_last();
	bool empty() const;

	/**
	 * Append the given name to this namespace.
	 * Return the combined fqon.
	 * Skip n components of the given name before appending.
	 */
	fqon_t combine(const IDToken &name, size_t skip=0) const;

	std::string to_filename() const;
	static Namespace from_filename(const std::string &filename);

	fqon_t to_fqon() const;

	std::string str() const;

	bool operator ==(const Namespace &other) const;

protected:
	std::vector<std::string> components;
};

} // namespace nyan


namespace std {
template <>
struct hash<nyan::Namespace> {
	size_t operator ()(const nyan::Namespace &ns) const;
};
} // namespace std
