// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "ast.h"

#include <iostream>
#include <sstream>

#include "compiler.h"
#include "config.h"
#include "ops.h"


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
				throw InternalError{"some token came after EOF."};
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
	comma_list(
		token_type::RBRACKET,
		tokens,
		[this] (const Token &token, TokenStream &stream) {
			stream.reinsert(&token);
			this->inheritance_change.push_back(ASTInheritanceChange{stream});
		}
	);
}


void ASTObject::ast_parents(TokenStream &tokens) {
	comma_list(
		token_type::RPAREN,
		tokens,
		[this] (const Token &token, TokenStream &stream) {

			if (token.type != token_type::ID) {
				throw ASTError{
					"expected inheritance parent identifier, but there is", token
				};
			}

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

		token = tokens.next();
	}
}


const Token &ASTObject::get_name() const {
	return this->name;
}


const std::vector<ASTObject> &ASTObject::get_objects() const {
	return this->objects;
}


ASTInheritanceChange::ASTInheritanceChange(TokenStream &tokens) {

	bool had_operator = false;
	bool had_target = false;
	auto token = tokens.next();

	if (token->type == token_type::OPERATOR) {
		had_operator = true;
		nyan_op action = op_from_token(*token);

		switch (action) {
		case nyan_op::ADD:
			this->type = inher_change_t::ADD_BACK;
			break;
		default:
			throw ASTError{"unsupported inheritance change operator", *token};
		}
		token = tokens.next();
	}

	if (token->type == token_type::ID) {
		this->target = IDToken{*token, tokens};
		had_target = true;

		token = tokens.next();
	}

	if (unlikely(not (had_operator or had_target))) {
		throw ASTError{"expected inheritance operator or identifier, there is", *token};
	}

	if (token->type == token_type::OPERATOR) {
		if (unlikely(had_operator)) {
			throw ASTError{
				"inheritance modifier already had operator at front", *token, false
			};
		}

		had_operator = true;
		nyan_op action = op_from_token(*token);

		switch (action) {
		case nyan_op::ADD:
			this->type = inher_change_t::ADD_FRONT;
			break;
		default:
			throw ASTError{"unsupported inheritance change operator", *token};
		}
		token = tokens.next();
	}

	if (unlikely(not had_operator)) {
		throw ASTError{"inheritance change is missing operator", *token, false};
	}
	else {
		tokens.reinsert(token);
	}
}


inher_change_t ASTInheritanceChange::get_type() const {
	return this->type;
}


const IDToken &ASTInheritanceChange::get_target() const {
	return this->target;
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
		auto next_token = tokens.next();

		// look ahead if this is a set type configuration
		if (not token->is_endmarker() and
		    next_token->type == token_type::LBRACE) {

			container_t ctype;
			if (token->get() == "o") {
				ctype = container_t::ORDEREDSET;
			}
			else {
				throw ASTError{"unhandled set type", *token};
			}

			this->value = ASTMemberValue{ctype, tokens};
		}
		else {
			tokens.reinsert(next_token);

			if (token->type == token_type::LBRACE) {
				// no set type defined => it's a standard set
				this->value = ASTMemberValue{container_t::SET, tokens};
			}
			else {
				// single-value

				if (unlikely(not token->is_content())) {
					throw ASTError{"expected value, have", *token};
				}

				this->value = ASTMemberValue{IDToken{*token, tokens}};
			}
		}

		had_def_or_decl = true;

		token = tokens.next();
	}
	if (not had_def_or_decl) {
		throw ASTError("expected type declaration ( : type ) "
		               "or value ( = something), instead got",
		               *token);
	}

	if (not token->is_endmarker()) {
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
	case container_t::ORDEREDSET:
		end_token = token_type::RBRACE; break;

	default:
		throw InternalError{"unknown container value type"};
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


	if (this->inheritance_change.size() > 0) {
		builder << "[";
		for (auto &change : this->inheritance_change) {
			change.strb(builder);
		}
		builder << "]";
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


void ASTInheritanceChange::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	switch (this->type) {
	case inher_change_t::ADD_BACK:
		builder << "+";
		break;
	default:
		break;
	}

	builder << this->target.str();

	switch (this->type) {
	case inher_change_t::ADD_FRONT:
		builder << "+";
		break;
	default:
		break;
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
		builder << "o{"; break;

	default:
		throw InternalError{"unhandled container type"};
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
	case container_t::ORDEREDSET:
		builder << "}"; break;

	default:
		throw InternalError{"unhandled container type"};
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
