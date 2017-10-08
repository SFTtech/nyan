// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "namespace_finder.h"

#include <sstream>

#include "meta_info.h"


namespace nyan {

NamespaceFinder::NamespaceFinder(AST &&ast)
	:
	ast{std::move(ast)} {}


void NamespaceFinder::add_import(const Namespace &import) {
	this->imports.insert(import);
}


void NamespaceFinder::add_alias(const Token &alias,
                                const Namespace &destination) {

	const std::string &search = alias.get();

	if (this->aliases.find(search) != std::end(this->aliases)) {
		throw NameError{alias, "redefinition of namespace alias", search};
	}

	this->aliases.insert({search, destination});
}


bool NamespaceFinder::check_conflict(const std::string &name) const {
	return (this->aliases.find(name) != std::end(this->aliases) or
	        this->imports.find(Namespace{name}) != std::end(this->imports));
}


fqon_t NamespaceFinder::expand_alias(const IDToken &name) const {
	if (unlikely(not name.exists())) {
		throw InternalError{"tried expanding alias on empty id token"};
	}

	// only the first component can be an alias.
	const std::string &first = name.get_components()[0].get();

	auto it = this->aliases.find(first);
	if (it != std::end(this->aliases)) {
		// alias found. now expand it.
		return it->second.combine(name, 1);
	}

	// no alias found. basically return the input name.
	return Namespace{name}.to_fqon();
}


fqon_t NamespaceFinder::find(const Namespace &current,
                             const IDToken &search,
                             const MetaInfo &typedb) const {

	if (unlikely(not search.exists())) {
		throw InternalError{"tried to find namespace for empty id"};
	}

	Namespace search_base{current};

	fqon_t result;
	// go towards the root namespace
	// to search for the matching object name
	while (true) {
		result = search_base.combine(search);
		if (typedb.has_object(result)) {
			return result;
		}

		// if the search base is exhausted, do alias expansion.
		if (search_base.empty()) {
			result = this->expand_alias(search);

			if (not typedb.has_object(result)) {
				throw NameError{search, "unknown name", search.str()};
			}

			return result;
		}

		search_base.pop_last();
	}
}


const AST &NamespaceFinder::get_ast() const {
	return this->ast;
}


std::string NamespaceFinder::str() const {
	std::ostringstream builder;
	builder << "NamespaceFinder knows:" << std::endl
	        << "= aliases:" << std::endl;

	for (auto &it : this->aliases) {
		builder << " * " << it.first
		        << " => " << it.second.str() << std::endl;
	}

	builder << "= imports:" << std::endl;

	for (auto &it : this->imports) {
		builder << " * " << it.str() << std::endl;
	}

	return builder.str();
}

} // namespace nyan
