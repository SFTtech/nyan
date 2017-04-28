// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_PARSER_H_
#define NYAN_PARSER_H_

#include <vector>

#include "ast.h"
#include "token.h"


namespace nyan {

class Database;
class File;
class Member;
class Object;
class TypeContainer;
class ValueContainer;

/**
 * The parser for nyan.
 */
class Parser {
public:
	Parser(Database *database);
	virtual ~Parser() = default;

	std::vector<Object *> parse(const File &file);

protected:
	/**
	 * Create the token stream from a file.
	 */
	std::vector<Token> tokenize(const File &file) const;

	/**
	 * Create the abstact syntax tree from a token stream.
	 */
	AST create_ast(const std::vector<Token> &tokens) const;

	/**
	 * Create nyan objects and place them in the database
	 * (which was specified in the constructor)
	 */
	std::vector<Object *> create_objects(const AST &ast);

	/**
	 * Add the object inheritance to an object to be constructed
	 * from that AST part.
	 */
	void add_inheritance(Object *obj, const ASTObject &astobj) const;

	/**
	 * Add the patch target objects from the AST to the object.
	 */
	void add_patch_targets(Object *obj, const ASTObject &astobj) const ;

	/**
	 * Determine the types of members, optionally consult parent objects
	 * in the database to get the type.
	 */
	std::unordered_map<std::string, TypeContainer> member_type_creation(Object *obj, const ASTObject &astobj) const;

	/**
	 * Create member entries which can then be stored in an object.
	 */
	std::vector<std::unique_ptr<Member>> create_members(Object *obj, const ASTObject &astobj, std::unordered_map<std::string, TypeContainer> &member_types) const;

	/**
	 * Create a Value from an AST member value.
	 * Check if the ast value can be assigned (with the given operation)
	 * to the member type determined already.
	 */
	ValueContainer create_member_value(const Type *member_type, const ASTMemberValue &astmembervalue) const;

	/**
	 * Create a Value from a single value token.
	 */
	ValueContainer value_from_value_token(const Token &value_token) const;

	/**
	 * Store the inheritance modifications of a patch.
	 */
	void inheritance_mod(Object *obj, const ASTObject &astobj) const ;

	/**
	 * The database where the parser will add resulting data to.
	 */
	Database *database;
};

} // namespace nyan

#endif
