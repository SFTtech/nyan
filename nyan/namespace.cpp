// Copyright 2016-2023 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "namespace.h"


#include <algorithm>

#include "api_error.h"
#include "id_token.h"
#include "util.h"

namespace nyan {

static const std::string extension = ".nyan";

Namespace::Namespace(std::vector<std::string> &&dir_components,
                     std::string &&filename,
                     std::vector<std::string> &&obj_components) :
	dir_components{std::move(dir_components)},
	filename{std::move(filename)},
	obj_components{std::move(obj_components)} {
	// If we have objects, there needs to be a file that contains them.
	if (not this->obj_components.empty() and this->filename.empty()) [[unlikely]] {
		throw InternalError{"Namespace with object components requires filename component"};
	}
}

Namespace::Namespace(const std::vector<std::string> &dir_components,
                     const std::string &filename,
                     const std::vector<std::string> &obj_components) :
	dir_components{dir_components},
	filename{filename},
	obj_components{obj_components} {
}

Namespace::Namespace(const Namespace &other,
                     const std::string &obj_addend) :
	Namespace{other} {
	if (this->is_dir()) {
		throw InternalError{"Cannot add object components to directory namespace"};
	}

	this->obj_components.push_back(obj_addend);
}

void Namespace::pop_last() {
	if (this->empty()) {
		throw InternalError{"popping from empty namespace"};
	}

	if (not this->obj_components.empty()) {
		this->obj_components.pop_back();
	}
	else if (not this->filename.empty()) {
		this->filename.clear();
	}
	else {
		this->dir_components.pop_back();
	}
}

bool Namespace::empty() const {
	return this->dir_components.empty()
	       and this->filename.empty()
	       and this->obj_components.empty();
}

fqon_t Namespace::combine(const IDToken &name, size_t skip) const {
	Namespace combined{*this};

	// append all components, but skip the first n parts.
	for (auto &part : name.get_components()) {
		if (skip > 0) {
			skip -= 1;
		}
		else {
			combined.obj_components.push_back(part.get());
		}
	}

	return combined.to_fqon();
}

bool Namespace::is_dir() const {
	return (this->filename.empty() and this->obj_components.empty());
}

bool Namespace::is_file() const {
	return not this->filename.empty() and this->obj_components.empty();
}

bool Namespace::is_obj() const {
	return not this->obj_components.empty();
}

bool Namespace::is_nested_obj() const {
	return this->obj_components.size() > 1;
}

const std::vector<std::string> &Namespace::get_dir_components() const {
	return this->dir_components;
}

const std::string &Namespace::get_filename() const {
	return this->filename;
}

const std::vector<std::string> &Namespace::get_obj_components() const {
	return this->obj_components;
}

std::string Namespace::to_dirpath() const {
	return util::strjoin(
		"/",
		this->dir_components
	);
}

std::string Namespace::to_filepath() const {
	std::string ret = util::strjoin(
		"/",
		this->dir_components
	);

	if (not this->filename.empty()) {
		ret += "/" + this->filename + extension;
	}

	return ret;
}

fqon_t Namespace::to_fqon() const {
	std::string ret;
	if (not this->dir_components.empty()) {
		ret += util::strjoin(
			".",
			this->dir_components,
			[](const auto &in) -> const std::string & {
				return in;
			});
	}

	if (not this->filename.empty()) {
		if (not ret.empty()) {
			ret += ".";
		}
		ret += this->filename;
	}

	if (not this->obj_components.empty()) {
		ret += "." + util::strjoin(".", this->obj_components);
	}

	return ret;
}

std::string Namespace::str() const {
	return this->to_fqon();
}

bool Namespace::operator==(const Namespace &other) const {
	return this->dir_components == other.dir_components
	       and this->filename == other.filename
	       and this->obj_components == other.obj_components;
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

	// strip file extension
	std::string namespace_name = filename.substr(0, filename.size() - extension.size());

	// sanitize the filename (remove empty components)
	// TODO: Do this via a file API
	std::vector<std::string> components{};
	for (auto &comp : util::split(namespace_name, '/')) {
		if (not comp.empty()) {
			components.push_back(comp);
		}
	}

	std::vector<std::string> dir_components{components.begin(), components.end() - 1};
	std::string file_component = components.back();

	// the fqon_t constructor.
	return Namespace{std::move(dir_components), std::move(file_component)};
}

} // namespace nyan


namespace std {

size_t hash<nyan::Namespace>::operator()(const nyan::Namespace &ns) const {
	size_t ret = 0;
	for (auto &component : ns.dir_components) {
		ret = nyan::util::hash_combine(ret, std::hash<std::string>{}(component));
	}

	ret = nyan::util::hash_combine(ret, std::hash<std::string>{}(ns.filename));

	for (auto &component : ns.obj_components) {
		ret = nyan::util::hash_combine(ret, std::hash<std::string>{}(component));
	}

	return ret;
}

} // namespace std
