// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_AST_H_
#define NYAN_NYAN_AST_H_

#include <string>
#include <sstream>
#include <vector>

#include "nyan_error.h"
#include "nyan_ops.h"
#include "nyan_token.h"
#include "nyan_util.h"

namespace nyan {

/**
 * Base class for nyan AST classes.
 */
class NyanASTBase {
	friend class NyanParser;
public:
	/**
	 * Return a string representation of this AST element
	 * and maybe its children.
	 */
	std::string str();

protected:
	virtual void strb(std::ostringstream &builder) = 0;
};


/**
 * The abstract syntax tree representation of a member entry.
 */
class NyanASTMember : public NyanASTBase {
public:
	NyanASTMember(const std::string &name, util::Iterator<NyanToken> &tokens);
	virtual ~NyanASTMember() = default;

	void strb(std::ostringstream &builder) override;

protected:
	std::string name;
	nyan_op operation;
	std::string type;
	std::string value;
};

/**
 * The abstract syntax tree representation of a nyan object.
 */
class NyanASTObject : public NyanASTBase {
public:
	NyanASTObject(const std::string &name, util::Iterator<NyanToken> &tokens);
	virtual ~NyanASTObject() = default;

	void ast_targets(util::Iterator<NyanToken> &tokens);
	void ast_inheritance_mod(util::Iterator<NyanToken> &tokens);
	void ast_inheritance(util::Iterator<NyanToken> &tokens);
	void ast_members(util::Iterator<NyanToken> &tokens);

	void strb(std::ostringstream &builder) override;

protected:
	std::string name;
	std::vector<std::string> targets;
	std::vector<std::string> inheritance_add;
	std::vector<std::string> inheritance;
	std::vector<NyanASTMember> members;
};

/**
 * Abstract syntax tree root.
 */
class NyanAST : public NyanASTBase {
public:
	NyanAST(util::Iterator<NyanToken> &tokens);
	virtual ~NyanAST() = default;

	void strb(std::ostringstream &builder) override;

protected:
	std::vector<NyanASTObject> objects;
};


/**
 * AST creation failure
 */
class ASTError : public ParserError {
public:
	ASTError(const std::string &msg, int line, int line_offset);
	ASTError(const std::string &msg, const NyanToken &token);
	virtual ~ASTError() = default;
};


/**
 * Add token values to the returned vector until the end token is
 * encountered.
 */
std::vector<std::string> comma_list(util::Iterator<NyanToken> &tokens,
                                    token_type end);


} // namespace nyan

#endif
