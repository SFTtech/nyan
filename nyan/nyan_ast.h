// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_AST_H_
#define NYAN_NYAN_AST_H_

#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "nyan_error.h"
#include "nyan_location.h"
#include "nyan_ops.h"
#include "nyan_token.h"
#include "nyan_type.h"
#include "nyan_util.h"

namespace nyan {


/**
 * Base class for nyan AST classes.
 */
class NyanASTBase {
	friend class NyanParser;
public:
	virtual ~NyanASTBase() = default;

	/**
	 * Return a string representation of this AST element
	 * and maybe its children.
	 */
	std::string str() const;

	/**
	 * Add token values to the returned vector until the end token is
	 * encountered.
	 */
	std::vector<NyanToken> comma_list(util::Iterator<NyanToken> &tokens,
	                                  token_type end) const;

protected:
	virtual void strb(std::ostringstream &builder) const = 0;
};


/**
 * AST representation of a member type declaration.
 */
class NyanASTMemberType : NyanASTBase {
	friend class NyanASTMember;
	friend class NyanParser;
	friend class NyanType;
public:
	NyanASTMemberType();
	NyanASTMemberType(const NyanToken &name, util::Iterator<NyanToken> &tokens);

	void strb(std::ostringstream &builder) const override;

protected:
	bool exists;
	NyanToken name;
	bool has_payload;
	NyanToken payload;
};


/**
 * AST representation of a member value.
 */
class NyanASTMemberValue : public NyanASTBase {
	friend class NyanParser;
	friend class NyanASTMember;

public:
	NyanASTMemberValue();
	NyanASTMemberValue(nyan_container_type type,
	                   util::Iterator<NyanToken> &tokens);
	NyanASTMemberValue(const NyanToken &token);

	void strb(std::ostringstream &builder) const override;

protected:
	bool exists;
	nyan_container_type container_type;

	std::vector<NyanToken> values;
};


/**
 * The abstract syntax tree representation of a member entry.
 */
class NyanASTMember : public NyanASTBase {
	friend class NyanParser;
public:
	NyanASTMember(const NyanToken &name, util::Iterator<NyanToken> &tokens);

	void strb(std::ostringstream &builder) const override;

protected:
	NyanToken name;
	nyan_op operation;
	NyanASTMemberType type;
	NyanASTMemberValue value;
};


/**
 * The abstract syntax tree representation of a nyan object.
 */
class NyanASTObject : public NyanASTBase {
	friend class NyanParser;
public:
	NyanASTObject(const NyanToken &name, util::Iterator<NyanToken> &tokens);

	void ast_targets(util::Iterator<NyanToken> &tokens);
	void ast_inheritance_mod(util::Iterator<NyanToken> &tokens);
	void ast_inheritance(util::Iterator<NyanToken> &tokens);
	void ast_members(util::Iterator<NyanToken> &tokens);

	void strb(std::ostringstream &builder) const override;

protected:
	NyanToken name;
	NyanToken target;
	std::vector<NyanToken> inheritance_add;
	std::vector<NyanToken> inheritance;
	std::vector<NyanASTMember> members;
};


/**
 * Abstract syntax tree root.
 */
class NyanAST : public NyanASTBase {
	friend class NyanParser;
public:
	NyanAST(util::Iterator<NyanToken> &tokens);

	void strb(std::ostringstream &builder) const override;
	const std::vector<NyanASTObject> &get_objects() const;

protected:
	std::vector<NyanASTObject> objects;
};


/**
 * AST creation failure
 */
class ASTError : public NyanFileError {
public:
	ASTError(const std::string &msg, const NyanToken &token,
	         bool add_token=true);
};

} // namespace nyan

#endif
