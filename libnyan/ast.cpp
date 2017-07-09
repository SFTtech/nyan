// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "ast.h"

#include <iostream>
#include <sstream>

#include "config.h"
#include "ops.h"


using namespace std::string_literals;

namespace nyan {


void comma_list(token_type end,
                TokenStream &tokens,
                const std::function<void(const Token &, TokenStream &)> &func) {

	auto token = tokens.next();
	bool comma_expected = false;

	// add identifiers until the expected end token is reached.
	while (true) {
		if (token->type == token_type::ENDLINE) {
			token = tokens.next();
			continue;
		}
		else if (token->type == end) {
			break;
		}
		else if (token->type == token_type::COMMA) {
			if (comma_expected) {
				token = tokens.next();
				comma_expected = false;
				continue;
			}
			else {
				throw ASTError{"expecting value, but got", *token};
			}
		}
		else if (comma_expected) {
			throw ASTError{"expected comma, but got", *token};
		}

		// value encountered, now process it.
		func(*token, tokens);

		comma_expected = true;

		// now the container is over, or a comma must follow
		token = tokens.next();
	}
}


std::string ASTBase::str() const {
	std::ostringstream builder;
	this->strb(builder);
	return builder.str();
}


const std::vector<ASTObject> &AST::get_objects() const {
	return this->objects;
}


const std::vector<ASTImport> &AST::get_imports() const {
	return this->imports;
}


AST::AST(TokenStream &tokens) {
	while (tokens.full()) {
		auto token = tokens.next();
		if (token->type == token_type::IMPORT) {
			this->imports.push_back(ASTImport{tokens});
		}
		else if (token->type == token_type::ID) {
			this->objects.push_back(ASTObject{*token, tokens});
		}
		else if (token->type == token_type::ENDFILE) {
			// we're done!
			if (tokens.empty()) {
				return;
			}
			else {
				throw Error{"some token came after EOF."};
			}
		}
		else {
			throw ASTError{"expected object name, but got", *token};
		}
	}
}


ASTImport::ASTImport(TokenStream &tokens) {
	auto token = tokens.next();

	if (token->type == token_type::ID) {
		this->namespace_name = IDToken{*token, tokens};
	} else {
		throw ASTError("expected namespace name to import, encountered", *token);
	}

	token = tokens.next();
	if (token->type == token_type::AS) {
		token = tokens.next();

		if (token->type != token_type::ID) {
			throw ASTError{"expected namespace alias identifier, but encountered", *token};
		}
		this->alias = *token;
		token = tokens.next();
	}

	if (token->type != token_type::ENDLINE) {
		throw ASTError("newline expected after import, there is", *token);
	}
}

const IDToken &ASTImport::get() const {
	return this->namespace_name;
}


bool ASTImport::has_alias() const {
	return (this->alias.get().size() > 0);
}


const Token &ASTImport::get_alias() const {
	return this->alias;
}


ASTObject::ASTObject(const Token &name,
                     TokenStream &tokens)
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
		this->ast_parents(tokens);
	} else {
		throw ASTError("create the object with (), i got", *token);
	}

	token = tokens.next();
	if (token->type != token_type::COLON) {
		throw ASTError("expected a ':' but instead encountered", *token);
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


void ASTObject::ast_targets(TokenStream &tokens) {
	auto token = tokens.next();
	if (token->type == token_type::ID) {
		this->target = IDToken{*token, tokens};
	}
	else {
		throw ASTError("expected identifier, encountered", *token);
	}

	token = tokens.next();

	if (token->type != token_type::RANGLE) {
		throw ASTError("expected > as patch target end, there is", *token);
	}
}


void ASTObject::ast_inheritance_mod(TokenStream &tokens) {
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

		if (token->type != token_type::OPERATOR) {
			throw ASTError("expected operator, encountered", *token);
		}

		auto inheritance_op = token;
		nyan_op action = op_from_token(*inheritance_op);

		// currently, only inheritance parent adding is supported
		if (action != nyan_op::ADD) {
			throw ASTError{
				"expected + operator,"
				"instead got: '"s + op_to_string(action) +
				"' when using",
				*inheritance_op
			};
		}

		token = tokens.next();

		if (token->type != token_type::ID) {
			throw ASTError("expected identifier, encountered", *token);
		}

		switch (action) {
		case nyan_op::ADD:
			this->inheritance_add.push_back(IDToken{*token, tokens});
			break;
		default:
			std::cout << inheritance_op->str() << std::endl;
			throw ASTError{
				"unknown inheritance modification"s + op_to_string(action),
				*inheritance_op,
				false
			};
		}

		expect_comma = true;
		token = tokens.next();
	}
}


void ASTObject::ast_parents(TokenStream &tokens) {
	auto token = tokens.next();
	if (token->type == token_type::RPAREN) {
		return;
	}

	if (token->type == token_type::ENDLINE) {
		token = tokens.next();
	}

	if (token->type != token_type::ID) {
		throw ASTError("expected inheritance parent identifier, but there is", *token);
	}

	tokens.reinsert(token);

	comma_list(
		token_type::RPAREN,
		tokens,
		[this] (const Token &token, TokenStream &stream) {
			this->parents.push_back(IDToken{token, stream});
		}
	);
}


void ASTObject::ast_members(TokenStream &tokens) {
	auto token = tokens.next();

	while (token->type != token_type::DEDENT and
	       token->type != token_type::ENDFILE) {

		// content entry of the object
		if (token->type == token_type::ID) {

			// determine if this is a member or a nested object.
			bool object_next = false;
			auto lookahead = tokens.next();

			if (lookahead->type == token_type::OPERATOR or
			    lookahead->type == token_type::COLON) {

				object_next = false;
			}
			else if (lookahead->type == token_type::LANGLE or
			    lookahead->type == token_type::LBRACKET or
			    lookahead->type == token_type::LPAREN) {

				object_next = true;
			}
			else {
				// don't think this will ever happen, right?
				throw ASTError("could not identify member or nested object defintion "
				               "after", *token);
			}

			tokens.reinsert(lookahead);

			if (object_next) {
				this->objects.push_back(ASTObject{*token, tokens});
			}
			else {
				this->members.push_back(ASTMember{*token, tokens});
			}
		}
		else if (token->type == token_type::PASS or
		         token->type == token_type::ELLIPSIS) {
			// "empty" member entry.
			token = tokens.next();
			if (token->type != token_type::ENDLINE) {
				throw ASTError("expected newline after pass or '...', "
				               "but got", *token);
			}
		}
		else {
			throw ASTError("expected member or object identifier, "
			               "instead got", *token);
		}

		if (tokens.empty()) {
			throw Error{"reached end of file!"};
		}

		token = tokens.next();
	}
}


const Token &ASTObject::get_name() const {
	return this->name;
}


const std::vector<ASTObject> &ASTObject::get_objects() const {
	return this->objects;
}



ASTMember::ASTMember(const Token &name,
                     TokenStream &tokens) {

	this->name = IDToken{name, tokens};

	auto token = tokens.next();
	bool had_def_or_decl = false;

	// type specifier (the ": typename" part)
	if (token->type == token_type::COLON) {
		token = tokens.next();

		if (token->type == token_type::ID) {
			this->type = ASTMemberType{*token, tokens};
			had_def_or_decl = true;
		} else {
			throw ASTError{"expected type name, instead got", *token};
		}

		token = tokens.next();
	}

	// value assigning
	if (token->type == token_type::OPERATOR) {
		this->operation = op_from_token(*token);

		if (this->operation == nyan_op::INVALID) {
			throw ASTError{"invalid operation", *token};
		}

		token = tokens.next();

		if (token->type == token_type::LANGLE or
		    token->type == token_type::LBRACE) {

			// multi-value container
			container_t ctype;
			switch (token->type) {
			case token_type::LANGLE:
				ctype = container_t::ORDEREDSET; break;
			case token_type::LBRACE:
				ctype = container_t::SET; break;
			default:
				throw Error{"unhandled multi value container type"};
			}

			this->value = ASTMemberValue{ctype, tokens};
		}
		else {
			// single-value
			this->value = ASTMemberValue{IDToken{*token, tokens}};
		}

		had_def_or_decl = true;

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


ASTMemberType::ASTMemberType()
	:
	does_exist{false},
	has_payload{false} {}


ASTMemberType::ASTMemberType(const Token &name,
                             TokenStream &tokens)
	:
	does_exist{true},
	has_payload{false} {

	this->name = IDToken{name, tokens};

	// now there may follow a type payload, e.g. set(payloadtype)
	auto token = tokens.next();
	if (token->type == token_type::LPAREN) {
		token = tokens.next();
		if (token->type == token_type::ID) {
			this->payload = IDToken{*token, tokens};
			this->has_payload = true;
		}
		else {
			throw ASTError("expected type identifier, but got", *token);
		}

		token = tokens.next();

		if (token->type != token_type::RPAREN) {
			throw ASTError("expected closing parens, but encountered", *token);
		}
	} else {
		tokens.reinsert(token);
	}
}


bool ASTMemberType::exists() const {
	return this->does_exist;
}


ASTMemberValue::ASTMemberValue()
	:
	does_exist{false} {}


ASTMemberValue::ASTMemberValue(const IDToken &value)
	:
	does_exist{true},
	container_type{container_t::SINGLE} {

	this->values.push_back(value);
}


ASTMemberValue::ASTMemberValue(container_t type,
                               TokenStream &tokens)
	:
	does_exist{true},
	container_type{type} {

	token_type end_token;

	switch (this->container_type) {
	case container_t::SET:
		end_token = token_type::RBRACE; break;
	case container_t::ORDEREDSET:
		end_token = token_type::RANGLE; break;
	case container_t::SINGLE:
		throw Error{"wrong constructor called for single-element container"};
	}

	comma_list(
		end_token,
		tokens,
		[this] (const Token &token, TokenStream &stream) {
			this->values.push_back(IDToken{token, stream});
		}
	);
}


bool ASTMemberValue::exists() const {
	// the size of this->values doesn't matter, as the value could be an empty set.
	return this->does_exist;
}


const std::vector<IDToken> &ASTMemberValue::get_values() const {
	return this->values;
}


const container_t &ASTMemberValue::get_container_type() const {
	return this->container_type;
}


static void indenter(std::ostringstream &builder, int indentlevel) {
	builder << std::string(SPACES_PER_INDENT * indentlevel, ' ');
}


void AST::strb(std::ostringstream &builder, int indentlevel) const {
	size_t count = 0;
	for (auto &obj : this->objects) {
		builder << std::endl;
		indenter(builder, indentlevel);
		builder << "# [object " << count << "]" << std::endl;
		obj.strb(builder, indentlevel);
		count += 1;
	}
}


void ASTImport::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	builder << "import " << this->namespace_name.str();
	if (this->has_alias()) {
		builder << " as " << this->get_alias().get();
	}
}


void ASTObject::strb(std::ostringstream &builder, int indentlevel) const {
	indenter(builder, indentlevel);
	builder << this->name.get();

	auto token_str = [](const auto &in) {
		return in.str();
	};

	// print <target>
	if (this->target.exists()) {
		builder << "<" << this->target.str() << ">";
	}


	if (this->inheritance_add.size() > 0) {
		builder << "[+"
		        << util::strjoin(", +", this->inheritance_add, token_str)
		        << "]";
	}

	builder << "("
	        << util::strjoin(", ", this->parents, token_str)
	        << "):"
	        << std::endl;

	if (this->objects.size() > 0) {
		for (auto &object : this->objects) {
			object.strb(builder, indentlevel + 1);
		}
	}
	if (this->members.size() > 0) {
		for (auto &member : this->members) {
			member.strb(builder, indentlevel + 1);
		}
	}
	else {
		indenter(builder, indentlevel + 1);
		builder << "pass" << std::endl;
	}
}


void ASTMember::strb(std::ostringstream &builder, int indentlevel) const {
	indenter(builder, indentlevel);
	builder << this->name.str();

	if (this->type.exists()) {
		builder << " : ";
		this->type.strb(builder);
	}

	if (this->value.exists()) {
		builder << " "
		        << op_to_string(this->operation)
		        << " ";

		this->value.strb(builder);
	}

	builder << std::endl;
}


void ASTMemberType::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	builder << this->name.str();

	if (this->has_payload) {
		builder << "(" << this->payload.str() << ")";
	}
}


void ASTMemberValue::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	switch (this->container_type) {
	case container_t::SINGLE:
		builder << this->values[0].str();
		return;

	case container_t::SET:
		builder << "{"; break;

	case container_t::ORDEREDSET:
		builder << "<"; break;

	default:
		throw Error{"unhandled container type"};
	}

	bool comma_active = false;
	for (auto &value : this->values) {
		if (comma_active) {
			builder << ", ";
		}
		builder << value.str();
		comma_active = true;
	}

	switch (this->container_type) {
	case container_t::SET:
		builder << "}"; break;

	case container_t::ORDEREDSET:
		builder << ">"; break;

	default:
		throw Error{"unhandled container type"};
	}
}


ASTError::ASTError(const std::string &msg,
                   const Token &token,
                   bool add_token)
	:
	FileError{Location{token}, ""} {

	if (add_token) {
		std::ostringstream builder;
		builder << msg << ": "
		        << token_type_str(token.type);
		this->msg = builder.str();
	}
	else {
		this->msg = msg;
	}
}


ASTError::ASTError(const std::string &msg,
                   const IDToken &token,
                   bool add_token)
	:
	FileError{Location{token}, ""} {

	if (add_token) {
		std::ostringstream builder;
		builder << msg << ": "
		        << token_type_str(token.get_type());
		this->msg = builder.str();
	}
	else {
		this->msg = msg;
	}
}

} // namespace nyan
