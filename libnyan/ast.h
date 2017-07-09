// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "error.h"
#include "id_token.h"
#include "location.h"
#include "ops.h"
#include "token.h"
#include "token_stream.h"
#include "type.h"
#include "util.h"

namespace nyan {


/**
 * Add tokens to the value list until the end token is reached.
 */
void comma_list(token_type end,
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
	 * and maybe its children.
	 */
	std::string str() const;

protected:
	virtual void strb(std::ostringstream &builder, int indentlevel=0) const = 0;
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

	bool exists() const;
	void strb(std::ostringstream &builder, int indentlevel=0) const override;

protected:
	bool does_exist;
	IDToken name;

	bool has_payload;
	IDToken payload;
};


/**
 * AST representation of a member value.
 */
class ASTMemberValue : public ASTBase {
	friend class Database;
	friend class ASTMember;

public:
	ASTMemberValue();
	ASTMemberValue(container_t type,
	               TokenStream &tokens);
	ASTMemberValue(const IDToken &value);

	bool exists() const;

	const std::vector<IDToken> &get_values() const;
	const container_t &get_container_type() const;

	void strb(std::ostringstream &builder, int indentlevel=0) const override;

protected:
	bool does_exist;
	container_t container_type;

	std::vector<IDToken> values;
};


/**
 * The abstract syntax tree representation of a member entry.
 */
class ASTMember : public ASTBase {
	friend class Database;
public:
	ASTMember(const Token &name, TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel=0) const override;

protected:
	IDToken name;
	nyan_op operation;
	ASTMemberType type;
	ASTMemberValue value;
};


/**
 * An import in a nyan file is represented by this AST entry.
 * Used for the `import ... (as ...)` statement.
 */
class ASTImport : public ASTBase {
public:
	ASTImport(TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel=0) const override;

	/** return the imported namespace name */
	const IDToken &get() const;

	/** return true if this import has defined an alias */
	bool has_alias() const;

	/** return the alias, if existing */
	const Token &get_alias() const;

protected:
	IDToken namespace_name;
	Token alias;
};


/**
 * The abstract syntax tree representation of a nyan object.
 */
class ASTObject : public ASTBase {
	friend class Database;
public:
	ASTObject(const Token &name, TokenStream &tokens);

	void ast_targets(TokenStream &tokens);
	void ast_inheritance_mod(TokenStream &tokens);
	void ast_parents(TokenStream &tokens);
	void ast_members(TokenStream &tokens);

	const Token &get_name() const;
	const std::vector<ASTObject> &get_objects() const;

	void strb(std::ostringstream &builder, int indentlevel=0) const override;

protected:
	Token name;
	IDToken target;
	std::vector<IDToken> inheritance_add;
	std::vector<IDToken> parents;
	std::vector<ASTMember> members;
	std::vector<ASTObject> objects;
};


/**
 * Abstract syntax tree root.
 */
class AST : public ASTBase {
public:
	AST(TokenStream &tokens);

	void strb(std::ostringstream &builder, int indentlevel=0) const override;
	const std::vector<ASTObject> &get_objects() const;
	const std::vector<ASTImport> &get_imports() const;

protected:
	std::vector<ASTImport> imports;
	std::vector<ASTObject> objects;
};


/**
 * AST creation failure
 */
class ASTError : public FileError {
public:
	ASTError(const std::string &msg, const Token &token,
	         bool add_token=true);

	ASTError(const std::string &msg, const IDToken &token,
	         bool add_token=true);
};

} // namespace nyan
