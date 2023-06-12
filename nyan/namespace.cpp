// Copyright 2016-2023 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "namespace.h"


#include <algorithm>

#include "api_error.h"
#include "id_token.h"
#include "util.h"

namespace nyan {

static const std::string extension = ".nyan";


Namespace::Namespace(const fqon_t &name)
	:
	components{util::split(name, '.')} {}


Namespace::Namespace(const IDToken &token) {
	this->components.reserve(token.get_components().size());
	for (auto &tok : token.get_components()) {
		this->components.push_back(tok.get());
	}
}


Namespace::Namespace(const Namespace &other, const std::string &addend)
	:
	Namespace{other} {

	for (const auto &component : util::split(addend, '.')) {
		this->components.push_back(component);
	}
}


void Namespace::pop_last() {
	if (this->empty()) {
		throw InternalError{"popping from empty namespace"};
	}
	this->components.pop_back();
}


bool Namespace::empty() const {
	return this->components.empty();
}


fqon_t Namespace::combine(const IDToken &name, size_t skip) const {
	Namespace combined{*this};

	// append all components, but skip the first n parts.
	for (auto &part : name.get_components()) {
		if (skip > 0) {
			skip -= 1;
		} else {
			combined.components.push_back(part.get());
		}
	}

	return combined.to_fqon();
}


// TODO: nested objects have components within one file.
//       separating via / is wrong for them.
std::string Namespace::to_filename() const {
	std::string ret = util::strjoin(
		"/",
		this->components,
		[] (const auto &in) -> const std::string& {
			return in;
		}
	);

	ret += extension;
	return ret;
}


Namespace Namespace::from_filename(const std::string &filename) {
	if (not util::ends_with(filename, extension)) {
		throw APIError{"invalid file extension"};
	}

	size_t n = std::count(filename.begin(), filename.end(), '.');

	// only the .nyan dot is allowed
	if (n > 1) {
		throw APIError{"there's too many dots in the path"};
	}

	// sanitize the filename
	// TODO: Do this via a file API
	std::string namespace_name;
	char prev_char;
	char cur_char;
	// condition strips off file extension
	for (size_t i = 0; i < filename.size() - extension.size(); ++i) {
		cur_char = filename[i];
		
		// slashes get replaced with dots
		if (cur_char == '/') {
			// strip multiple slashes
			if (prev_char == '/') {
				continue;
			}
			namespace_name += '.';
		}
		// normal chars get copied
		else {
			namespace_name += cur_char;
		}

		prev_char = cur_char;
	}

	// the fqon_t constructor.
	return Namespace{namespace_name};
}


fqon_t Namespace::to_fqon() const {
	return util::strjoin(".", this->components);
}


std::string Namespace::str() const {
	return this->to_fqon();
}


bool Namespace::operator ==(const Namespace &other) const {
	return this->components == other.components;
}

} // namespace nyan


namespace std {

size_t hash<nyan::Namespace>::operator ()(const nyan::Namespace &ns) const {
	size_t ret = 0;
	for (auto &component : ns.components) {
		ret = nyan::util::hash_combine(ret, std::hash<std::string>{}(component));
	}
	return ret;
}

} // namespace std
