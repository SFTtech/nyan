// Copyright 2017-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

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

	/**
	 * Imports another namespace into the finder.
	 *
	 * @param ns Namespace that should be imported.
	 */
	void add_import(const Namespace &ns);

	/**
	 * Imports another namespace into the finder by using an alias.
	 *
	 * @param alias Token with the name of the alias.
	 * @param ns Namespace that should be imported.
	 */
	void add_alias(const Token &alias, const Namespace &destination);

	/**
	 * Check if a name conflicts wth other names in the namespace, i.e. the
	 * name is already used.
	 *
	 * @return true if the name is already used, else false.
	 */
	bool check_conflict(const std::string &name) const;

	/**
	 * Get the identifier of a namespace associated with an alias.
	 *
	 * @param name IDToken with an alias as its first component.
	 *
	 * @return Identifier of the namespace.
	 */
	fqon_t expand_alias(const IDToken &name) const;

	/**
	 * Search for the object/member identifier of an object/member reference
	 * in a given namespace.
	 *
	 * @param name IDToken with an object/member reference.
	 *
	 * @return Identifier of the object/member.
	 */
	fqon_t find(const Namespace &current,
	            const IDToken &search,
	            const MetaInfo &typedb) const;

	/**
	 * Get the AST (abstract syntax tree) of this namespace.
	 *
	 * @return AST of this namespace.
	 */
	const AST &get_ast() const;

	/**
	 * Get the string representation of this namespace.
	 *
	 * @return String representation of this namespace.
	 */
	std::string str() const;

public:
	/**
	 * Abstract syntax tree of the namespace of this finder.
	 */
	AST ast;

	/**
	 * Directly imported namespaces.
	 */
	namespace_available_t imports;

	/**
	 * Namespaces imported by alias.
	 */
	namespace_alias_t aliases;
};

} // namespace nyan
