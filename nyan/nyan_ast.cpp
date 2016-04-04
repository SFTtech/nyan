// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_ast.h"

#include <iostream>
#include <sstream>

#include "nyan_ops.h"

using namespace std::string_literals;

namespace nyan {

std::string NyanASTBase::str() const {
	std::ostringstream builder;
	this->strb(builder);
	return builder.str();
}


const std::vector<NyanASTObject> &NyanAST::get_objects() const {
	return this->objects;
}


NyanAST::NyanAST(util::Iterator<NyanToken> &tokens) {
	while (tokens.full()) {
		auto token = tokens.next();
		if (token->type == token_type::ID) {
			this->objects.push_back(NyanASTObject(token->value, tokens));
		}
		else if (token->type == token_type::ENDFILE) {
			// we're done!
		}
		else {
			throw ASTError("expected object name, but got", *token);
		}
	}
}

NyanASTObject::NyanASTObject(const std::string &name,
                             util::Iterator<NyanToken> &tokens)
	:
	name{name} {

	auto token = tokens.next();

	if (token->type == token_type::LANGLE) {
		this->ast_targets(tokens);
		token = tokens.next();
	}

	if (token->type == token_type::LBRACKET) {
		this->ast_inheritance_mod(tokens);
		token = tokens.next();
	}

	if (token->type == token_type::LPAREN) {
		this->ast_inheritance(tokens);
	} else {
		throw ASTError("create the object with (), i got", *token);
	}

	token = tokens.next();
	if (token->type != token_type::COLON) {
		throw ASTError("expected a : but instead encountered", *token);
	}

	token = tokens.next();
	if (token->type != token_type::ENDLINE) {
		throw ASTError("expected a newline there is", *token);
	}

	token = tokens.next();
	if (token->type != token_type::INDENT) {
		throw ASTError("expected indentation but instead there's", *token);
	}

	this->ast_members(tokens);
}

void NyanASTObject::ast_targets(util::Iterator<NyanToken> &tokens) {
	this->targets = this->comma_list(tokens, token_type::RANGLE);
}

void NyanASTObject::ast_inheritance_mod(util::Iterator<NyanToken> &tokens) {
	bool expect_comma = false;
	auto token = tokens.next();

	while (token->type != token_type::RBRACKET) {
		if (token->type == token_type::COMMA) {
			expect_comma = false;
			token = tokens.next();
		}
		else if (expect_comma == true) {
			throw ASTError("expected comma, encountered", *token);
		}

		nyan_op action = op_from_token(*token);

		if (action != nyan_op::ADD) {
			throw ASTError("expected + operator,"
			               "instead got: '"s + token->value +
			               "' when using", *token);
		}
		token = tokens.next();

		// add parent
		if (token->type == token_type::ID) {
			if (action == nyan_op::ADD) {
				this->inheritance_add.push_back(token->value);
			}
			expect_comma = true;
		}
		else {
			throw ASTError("expected identifier, encountered", *token);
		}

		token = tokens.next();
	}
}

void NyanASTObject::ast_inheritance(util::Iterator<NyanToken> &tokens) {
	this->inheritance = this->comma_list(tokens, token_type::RPAREN);
}

void NyanASTObject::ast_members(util::Iterator<NyanToken> &tokens) {
	auto token = tokens.next();

	while (token->type != token_type::DEDENT and
	       token->type != token_type::ENDFILE) {

		if (token->type == token_type::ID) {
			this->members.push_back(NyanASTMember(token->value, tokens));
		}
		else if (token->type == token_type::PASS) {
			// "empty" member entry.
			token = tokens.next();
			if (token->type != token_type::ENDLINE and
			    token->type != token_type::ENDFILE) {
				throw ASTError("expected newline after pass, but got",
				               *token);
			}

		} else {
			throw ASTError("expected member identifier, but got", *token);
		}


		token = tokens.next();
	}
}


NyanASTMember::NyanASTMember(const std::string &name,
                             util::Iterator<NyanToken> &tokens)
	:
	name{name} {

	auto token = tokens.next();
	bool had_def_or_decl = false;

	// type specifier
	if (token->type == token_type::COLON) {
		token = tokens.next();
		if (token->type == token_type::ID) {
			this->type = token->value;
			had_def_or_decl = true;
		} else {
			throw ASTError("expected type name, instead got", *token);
		}
		token = tokens.next();
	}

	// value assigning
	if (token->type == token_type::OPERATOR) {
		this->operation = op_from_token(*token);

		token = tokens.next();

		if (token->type == token_type::ID or
		    token->type == token_type::INT or
		    token->type == token_type::FLOAT or
		    token->type == token_type::STRING) {

			this->value = token->value;
			had_def_or_decl = true;
		}
		else {
			throw ASTError("expected some value but there is", *token);
		}
		token = tokens.next();
	}
	else if (had_def_or_decl == false) {
		throw ASTError("expected type declaration ( : type ) "
		               "or value ( = something), instead got",
		               *token);
	}

	if (token->type != token_type::ENDLINE and
	    token->type != token_type::ENDFILE) {
		throw ASTError("expected newline after member entry, but got",
		               *token);
	}
}


std::vector<std::string> NyanASTBase::comma_list(
	util::Iterator<NyanToken> &tokens, token_type end) const {

	std::vector<std::string> ret;

	auto token = tokens.next();
	bool expect_comma = false;

	while (token->type != end) {
		if (token->type == token_type::COMMA) {
			expect_comma = false;
			token = tokens.next();
		}
		else if (expect_comma == true) {
			throw ASTError("expected comma, encountered", *token);
		}

		if (token->type == token_type::ID) {
			ret.push_back(token->value);
			expect_comma = true;
		}
		else {
			throw ASTError("expected identifier, encountered", *token);
		}
		token = tokens.next();
	}
	return ret;
}


void NyanAST::strb(std::ostringstream &builder) const {
	builder << "### nyan tree ###" << std::endl;

	size_t count = 0;
	for (auto &obj : this->objects) {
		builder << std::endl << "# [object " << count++ << "]" << std::endl;
		obj.strb(builder);
	}
}


void NyanASTObject::strb(std::ostringstream &builder) const {
	builder << this->name;

	// print <target, target, >
	if (this->targets.size() > 0) {
		builder << "<" << util::strjoin(", ", this->targets) << ">";
	}

	if (this->inheritance_add.size() > 0) {
		builder << "[+" << util::strjoin(", +", this->inheritance_add)
		        << "]";
	}

	builder << "(" << util::strjoin(", ", this->inheritance) << "):"
	        << std::endl;

	if (this->members.size() > 0) {
		for (auto &member : this->members) {
			builder << "    ";
			member.strb(builder);
		}
	}
	else {
		builder << "    pass" << std::endl;
	}
}


void NyanASTMember::strb(std::ostringstream &builder) const {
	builder << this->name;
	if (this->type.size() > 0) {
		builder << " : " << this->type;
	}

	if (this->value.size() > 0) {
		builder << " "
		        << op_to_string(this->operation)
		        << " " << this->value;
	}

	builder << std::endl;
}


ASTError::ASTError(const std::string &msg, int line, int line_offset)
	:
	ParserError{msg, line, line_offset} {}

ASTError::ASTError(const std::string &msg, const NyanToken &token)
	:
	ParserError{"", token.line, token.line_offset} {

		std::ostringstream builder;
		builder << msg << ": "
		        << token_type_str(token.type);
		this->msg = builder.str();
	}


} // namespace nyan
