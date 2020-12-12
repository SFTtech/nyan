// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "error.h"
#include "id_token.h"
#include "lang_error.h"
#include "location.h"
#include "ops.h"
#include "token.h"
#include "token_stream.h"
#include "type.h"
#include "util.h"
#include "value_token.h"

namespace nyan {

/**
 * Walks through a comma separated list and calls a user-provided
 * function on a list item. Stops when either the specified end token
 * is reached or the number of read items is equal to the set limit.
 *
 * @param end End delimiter of the comma seperated list.
 * @param tokens TokenStream that is walked through.
 * @param limit Maximum number of list items that should be processed.
 * @param func Function called on the list item.
 *
 * @return Number of list items processed.
 */
unsigned int comma_list(token_type end,
                        TokenStream &tokens,
                        unsigned int limit,
                        const std::function<void(const Token &, TokenStream &)> &func);


/**
 * Walks through a comma separated list and calls a user-provided
 * function on a list item. Stops when the specified end token
 * is reached.
 *
 * @param end End delimiter of the comma seperated list.
 * @param tokens TokenStream that is walked through.
 * @param func Function called on the list item.
 *
 * @return Number of list items processed.
 */
unsigned int comma_list(token_type end,
                        TokenStream &tokens,
                        const std::function<void(const Token &, TokenStream &)> &func);


/**
 * Base class for nyan AST classes.
 */
class ASTBase {
	friend class Database;

public:
	virtual ~ASTBase() = default;

	/**
	 * Return a string representation of this AST element
	 * and its children.
	 */
	std::string str() const;

protected:
	/**
	 * Append a string represantation of this AST element to a
	 * string stream. The element may indent the appended string.
	 *
	 * @param builder String stream that is appended to.
	 * @param indentlevel Indentation level (4 spaces per level).
	 */
	virtual void strb(std::ostringstream &builder, int indentlevel = 0) const = 0;
};


/**
 * AST representation of a member type argument declaration.
 */
class ASTMemberTypeArgument : public ASTBase {
	friend class ASTMemberType;
	friend class Database;
	friend class Type;

public:
	ASTMemberTypeArgument(TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

protected:
	/**
	 * true if the argument is a key-value pair.
	 */
	bool has_key;

	/**
	 * Argument key token. If the argument is not keyed, this is nullptr.
	 */
	IDToken key;

	/**
	 * Argument value token.
	 */
	IDToken value;
};


/**
 * AST representation of a member type declaration.
 */
class ASTMemberType : ASTBase {
	friend class ASTMember;
	friend class Database;
	friend class Type;

public:
	ASTMemberType();
	ASTMemberType(const Token &name, TokenStream &tokens);

	/**
	 * Checks if the member type was declared, i.e. it contains a type definition.
	 *
	 * @return true if the member type was declared, false if it
	 * wasn't. In the latter case the member type must be declared
	 * in a parent object.
	 */
	bool exists() const;
	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

protected:
	/**
	 * true if the member type was declared, i.e. it contains a type definition.
	 */
	bool does_exist;

	/**
	 * Typename of the member type.
	 */
	IDToken name;

	/**
	 * Nested types of a composite type member, e.g. int in set(int)
	 */
	std::vector<ASTMemberType> nested_types;

	/**
	 * true if the member type has at least one argument.
	 */
	bool has_args;

	/**
	 * Type arguments.
	 */
	std::vector<ASTMemberTypeArgument> args;
};


/**
 * AST representation of a member value.
 */
class ASTMemberValue : public ASTBase {
	friend class Database;
	friend class ASTMember;

public:
	ASTMemberValue();
	ASTMemberValue(composite_t type,
	               TokenStream &tokens);
	ASTMemberValue(const IDToken &value);

	/**
	 * Checks if the member value was defined, i.e. it contains values.
	 *
	 * @return true if the member value was defined, false if it
	 * wasn't. In the latter case the member is abstract.
	 */
	bool exists() const;

	/**
	 * Returns the values defined by this member value.
	 *
	 * @return A list of ValueToken objects that each contain a subvalue.
	 */
	const std::vector<ValueToken> &get_values() const;

	/**
	 * Returns the composite type of the member value.
	 *
	 * @return A composite type.
	 */
	const composite_t &get_composite_type() const;

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

protected:
	/**
	 * true if the member value was defined, i.e. it contains values.
	 */
	bool does_exist;

	/**
	 * Composite type of the member value. Defines how the value is formatted in the file.
	 */
	composite_t composite_type;

	/**
	 * Values defined in the member value.
	 */
	std::vector<ValueToken> values;
};


/**
 * The abstract syntax tree representation of a member entry.
 */
class ASTMember : public ASTBase {
	friend class Database;

public:
	ASTMember(const Token &name, TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

protected:
	/**
	 * Name (identifier) of the member.
	 */
	IDToken name;

	/**
	 * Operation defined by the member.
	 */
	nyan_op operation;

	/**
	 * Member type definition.
	 */
	ASTMemberType type;

	/**
	 * Member value definition.
	 */
	ASTMemberValue value;
};


/**
 * The abstract syntax tree representation of an argument.
 */
class ASTArgument : public ASTBase {
public:
	ASTArgument(TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

	/**
	 * Returns the argument type.
	 *
	 * @return IDToken with the argument type string.
	 */
	const IDToken &get_arg() const;

	/**
	 * Returns the argument parameters.
	 *
	 * @return A list of IDTokens with argument parameter values.
	 */
	const std::vector<IDToken> &get_params() const;

protected:
	IDToken arg;
	std::vector<IDToken> params;
};


/**
 * An import in a nyan file is represented by this AST entry.
 * Used for the `import ... (as ...)` statement.
 */
class ASTImport : public ASTBase {
public:
	ASTImport(TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

	/**
	 * Returns the imported namespace name.
	 *
	 * @return IDToken with the namespace name.
	 */
	const IDToken &get() const;

	/**
	 * Checks if the import has defined an alias.
	 *
	 * @return true if an alias is defined, else false.
	 */
	bool has_alias() const;

	/**
	 * Returns the import alias.
	 *
	 * @return The Token with the alias name, if it exists, or nullptr
	 * if it doesn't.
	 */
	const Token &get_alias() const;

protected:
	/**
	 * Name (identifier) of the namespace that is imported from.
	 */
	IDToken namespace_name;

	/**
	 * Import alias name.
	 */
	Token alias;
};


/**
 * The abstract syntax tree representation of an inheritance change.
 */
class ASTInheritanceChange : public ASTBase {
	friend class Database;

public:
	ASTInheritanceChange(TokenStream &tokens);

	/**
	 * Returns the inheritance change type.
	 *
	 * @return The inheritance change type.
	 */
	inher_change_t get_type() const;

	/**
	 * Returns the name of the object add to the inheritance.
	 *
	 * @return Inheritance change type.
	 */
	const IDToken &get_target() const;

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

protected:
	/**
	 * Determines at which end in the object's linearization the
	 * inheritance change is appended (front or back).
	 */
	inher_change_t type;

	/**
	 * Identifier of the object that is added to the inheritance.
	 */
	IDToken target;
};


/**
 * The abstract syntax tree representation of a nyan object.
 */
class ASTObject : public ASTBase {
	friend class Database;

public:
	ASTObject(const Token &name, TokenStream &tokens);

	/**
	 * Reads a patch target and sets it as 'target'.
	 *
	 * @param tokens TokenStream that points to the Token *before* the target begins.
	 */
	void ast_targets(TokenStream &tokens);

	/**
	 * Reads inheritance changes target and adds them to 'inheritance_change'.
	 *
	 * @param tokens TokenStream that points to first Token in the list of
	 * added inheritance objects.
	 */
	void ast_inheritance_mod(TokenStream &tokens);

	/**
	 * Reads object parents and adds them to 'parents'.
	 *
	 * @param tokens TokenStream that points to first Token in the list of
	 * parent objects.
	 */
	void ast_parents(TokenStream &tokens);

	/**
	 * Reads object members and adds them to 'parents'.
	 *
	 * @param tokens TokenStream that points to the Token *before* the members begin.
	 */
	void ast_members(TokenStream &tokens);

	/**
	 * Returns the object name.
	 *
	 * @return The Token with the object's name.
	 */
	const Token &get_name() const;

	/**
	 * Returns the nested objects in this object.
	 *
	 * @return A list of nested ASTObjects.
	 */
	const std::vector<ASTObject> &get_objects() const;

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

protected:
	/**
	 * Name of the object.
	 */
	Token name;

	/**
	 * Target if this is a patch.
	 */
	IDToken target;

	/**
	 * Inheritance changes to the target if this is a patch.
	 */
	std::vector<ASTInheritanceChange> inheritance_change;

	/**
	 * Parents of the object.
	 */
	std::vector<IDToken> parents;

	/**
	 * Members of the object.
	 */
	std::vector<ASTMember> members;

	/**
	 * Nested objects in the object.
	 */
	std::vector<ASTObject> objects;
};


/**
 * Abstract syntax tree root.
 */
class AST : public ASTBase {
public:
	AST(TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel = 0) const override;

	/**
	 * Returns the arguments in the AST.
	 *
	 * @return A list of ASTArguments.
	 */
	const std::vector<ASTArgument> &get_args() const;

	/**
	 * Returns the imports in the AST.
	 *
	 * @return A list of ASTImports.
	 */
	const std::vector<ASTImport> &get_imports() const;

	/**
	 * Returns the objects in the AST.
	 *
	 * @return A list of ASTObjects.
	 */
	const std::vector<ASTObject> &get_objects() const;

protected:
	/**
	 * Arguments in the AST.
	 */
	std::vector<ASTArgument> args;

	/**
	 * Imports in the AST.
	 */
	std::vector<ASTImport> imports;

	/**
	 * Objects in the AST.
	 */
	std::vector<ASTObject> objects;
};


/**
 * AST creation failure
 */
class ASTError : public LangError {
public:
	/**
	 * Creates an error for a Token in the parsers token stream.
	 *
	 * @param msg Error message string.
	 * @param token Token that caused the error.
	 * @param add_token If true, the token type is displayed.
	 */
	ASTError(const std::string &msg, const Token &token, bool add_token = true);

	/**
	 * Creates an error for an IDToken in the parsers token stream.
	 *
	 * @param msg Error message string.
	 * @param token IDToken that caused the error.
	 * @param add_token If true, the token type is displayed.
	 */
	ASTError(const std::string &msg, const IDToken &token, bool add_token = true);
};

} // namespace nyan
