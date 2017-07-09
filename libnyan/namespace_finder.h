// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ast.h"
#include "namespace.h"


namespace nyan {

class NamespaceFinder {
	/**
	 * Lookup map to find the namespace for an alias.
	 */
	using namespace_alias_t = std::unordered_map<std::string, Namespace>;

	/**
	 * In the current namespace, namespaces in here were imported.
	 */
	using namespace_available_t = std::unordered_set<Namespace>;

public:
	NamespaceFinder(AST &&ast);

	void add_import(const Namespace &ns);
	void add_alias(const Token &alias, const Namespace &destination);

	/**
	 * check for naming conflict of an object.
	 * returns true if the name conflicts.
	 */
	bool check_conflict(const std::string &name) const;

	/** Expand the id for an alias. */
	fqon_t expand_alias(const IDToken &name) const;

	/** Search for a fqon in a given namespace and below */
	fqon_t find(const Namespace &current,
	            const IDToken &search,
	            const MetaInfo &typedb) const;

	/** Return the AST of this namespace */
	const AST &get_ast() const;

	std::string str() const;

public:
	AST ast;

	namespace_available_t imports;

	namespace_alias_t aliases;
};

} // namespace nyan
