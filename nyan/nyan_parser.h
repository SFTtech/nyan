// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_PARSER_H_
#define NYAN_PARSER_H_

#include <vector>

#include "nyan_ast.h"
#include "nyan_token.h"
#include "nyan_type_container.h"
#include "nyan_value_container.h"


namespace nyan {

class NyanFile;
class NyanMember;
class NyanObject;
class NyanStore;

/**
 * The parser for nyan.
 */
class NyanParser {
public:
	NyanParser(NyanStore *store);
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
	 * Create nyan objects and place them in the store
	 * (which was specified in the constructor)
	 */
	std::vector<NyanObject *> create_objects(const NyanAST &ast);

	/**
	 * Add the object inheritance to an object to be constructed
	 * from that AST part.
	 */
	void add_inheritance(NyanObject *obj, const NyanASTObject &astobj) const;

	/**
	 * Determine the types of members, optionally consult parent objects
	 * in the store to get the type.
	 */
	std::unordered_map<std::string, NyanTypeContainer> member_type_creation(NyanObject *obj, const NyanASTObject &astobj) const;

	/**
	 * Create member entries which can then be stored in an object.
	 */
	std::vector<std::unique_ptr<NyanMember>> create_members(NyanObject *obj, const NyanASTObject &astobj, std::unordered_map<std::string, NyanTypeContainer> &member_types) const;

	/**
	 * Check if the ast value can be assigned (with the given operation)
	 * to the member type determined already.
	 */
	void check_member_value_type(const NyanTypeContainer &member_type, nyan_op member_operation, const NyanASTMemberValue &astmembervalue) const;

	/**
	 * Create a NyanValue from an AST member value.
	 */
	NyanValueContainer create_member_value(const NyanASTMemberValue &astmembervalue) const;

	/**
	 * Add the patch target objects from the AST to the object.
	 */
	void add_patch_targets(NyanObject *obj, const NyanASTObject &astobj) const ;

	/**
	 * Store the inheritance modifications of a patch.
	 */
	void inheritance_mod(NyanObject *obj, const NyanASTObject &astobj) const ;

	/**
	 * The store where the parser will add resulting data to.
	 */
	NyanStore *store;
};

} // namespace nyan

#endif
