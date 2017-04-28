// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_PARSER_H_
#define NYAN_PARSER_H_

#include <vector>

#include "ast.h"
#include "token.h"


namespace nyan {

class NyanDatabase;
class NyanFile;
class NyanMember;
class NyanObject;
class NyanTypeContainer;
class NyanValueContainer;

/**
 * The parser for nyan.
 */
class NyanParser {
public:
	NyanParser(NyanDatabase *database);
	virtual ~NyanParser() = default;

	std::vector<NyanObject *> parse(const NyanFile &file);

protected:
	/**
	 * Create the token stream from a file.
	 */
	std::vector<NyanToken> tokenize(const NyanFile &file) const;

	/**
	 * Create the abstact syntax tree from a token stream.
	 */
	NyanAST create_ast(const std::vector<NyanToken> &tokens) const;

	/**
	 * Create nyan objects and place them in the database
	 * (which was specified in the constructor)
	 */
	std::vector<NyanObject *> create_objects(const NyanAST &ast);

	/**
	 * Add the object inheritance to an object to be constructed
	 * from that AST part.
	 */
	void add_inheritance(NyanObject *obj, const NyanASTObject &astobj) const;

	/**
	 * Add the patch target objects from the AST to the object.
	 */
	void add_patch_targets(NyanObject *obj, const NyanASTObject &astobj) const ;

	/**
	 * Determine the types of members, optionally consult parent objects
	 * in the database to get the type.
	 */
	std::unordered_map<std::string, NyanTypeContainer> member_type_creation(NyanObject *obj, const NyanASTObject &astobj) const;

	/**
	 * Create member entries which can then be stored in an object.
	 */
	std::vector<std::unique_ptr<NyanMember>> create_members(NyanObject *obj, const NyanASTObject &astobj, std::unordered_map<std::string, NyanTypeContainer> &member_types) const;

	/**
	 * Create a NyanValue from an AST member value.
	 * Check if the ast value can be assigned (with the given operation)
	 * to the member type determined already.
	 */
	NyanValueContainer create_member_value(const NyanType *member_type, const NyanASTMemberValue &astmembervalue) const;

	/**
	 * Create a NyanValue from a single value token.
	 */
	NyanValueContainer value_from_value_token(const NyanToken &value_token) const;

	/**
	 * Store the inheritance modifications of a patch.
	 */
	void inheritance_mod(NyanObject *obj, const NyanASTObject &astobj) const ;

	/**
	 * The database where the parser will add resulting data to.
	 */
	NyanDatabase *database;
};

} // namespace nyan

#endif
