// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "ast.h"

#include <iostream>
#include <limits>
#include <sstream>

#include "compiler.h"
#include "config.h"
#include "ops.h"


namespace nyan {

unsigned int comma_list(token_type end,
                    	TokenStream &tokens,
			        	unsigned int limit,
                    	const std::function<void(const Token &, TokenStream &)> &func) {

	auto token = tokens.next();
	bool comma_expected = false;

	// add identifiers until the expected end token is reached.
	unsigned int index = 0;
	while (index < limit) {
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
		index++;

		// now the container is over, or a comma must follow
		token = tokens.next();
	}

	return index;
}


unsigned int comma_list(token_type end,
                    	TokenStream &tokens,
                    	const std::function<void(const Token &, TokenStream &)> &func) {
	return comma_list(end,
					  tokens,
					  std::numeric_limits<int>::max(),
					  func);
}


std::string ASTBase::str() const {
	std::ostringstream builder;
	this->strb(builder);
	return builder.str();
}


const std::vector<ASTArgument> &AST::get_args() const {
	return this->args;
}


const std::vector<ASTObject> &AST::get_objects() const {
	return this->objects;
}


const std::vector<ASTImport> &AST::get_imports() const {
	return this->imports;
}


AST::AST(TokenStream &tokens) {
	auto token = tokens.next();

	// Ensure that AST has version argument
	if (token->type == token_type::BANG) {
		this->args.emplace_back(tokens);

		if (this->args.front().get_arg().str() != "version") {
			throw InternalError{"file must start with 'version' argument, not "
								+ this->args.front().get_arg().str()};
		}
	}
	else {
		throw InternalError{"missing starting argument: version"};
	}

	while (tokens.full()) {
		token = tokens.next();
		if (token->type == token_type::BANG) {
			this->args.emplace_back(tokens);
		}
		else if (token->type == token_type::IMPORT) {
			this->imports.emplace_back(tokens);
		}
		else if (token->type == token_type::ID) {
			this->objects.emplace_back(*token, tokens);
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


ASTArgument::ASTArgument(TokenStream &tokens) {
	auto token = tokens.next();

	if (token->type == token_type::ID) {
		this->arg = IDToken{*token, tokens};
		token = tokens.next();
	} else {
		throw ASTError("expected argument keyword, encountered", *token);
	}

	while (not token->is_endmarker()) {
		if (token->is_content()) {
			this->params.emplace_back(*token, tokens);
			token = tokens.next();
		} else {
			throw ASTError("expected parameter value, encountered", *token);
		}
	}
}


const IDToken &ASTArgument::get_arg() const {
	return this->arg;
}


const std::vector<IDToken> &ASTArgument::get_params() const {
	return this->params;
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
		[this] (const Token & /*token*/, TokenStream &stream) {
			stream.reinsert_last();
			this->inheritance_change.emplace_back(stream);
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

			this->parents.emplace_back(token, stream);
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

			tokens.reinsert_last();

			if (object_next) {
				this->objects.emplace_back(*token, tokens);
			}
			else {
				this->members.emplace_back(*token, tokens);
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
		tokens.reinsert_last();
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

			composite_t ctype;
			if (token->get() == "o") {
				ctype = composite_t::ORDEREDSET;
			}
			else {
				throw ASTError{"unhandled set type", *token};
			}

			this->value = ASTMemberValue{ctype, tokens};
		}
		else {
			tokens.reinsert_last();

			if (token->type == token_type::LBRACE) {
				// default => it's a standard set
				composite_t ctype = composite_t::SET;

				// Look ahead to check if it's a dict
				// TODO: This is really inconvenient
				int look_ahead = 0;
				while (not (token->type == token_type::RBRACE
							or token->type == token_type::COMMA)) {
					token = tokens.next();
					look_ahead++;

					if (token->type == token_type::COLON) {
						ctype = composite_t::DICT;
						break;
					}
				}

				// Go back to start of container
				for (int i = look_ahead; i > 0; i--) {
					tokens.reinsert_last();
				}

				this->value = ASTMemberValue{ctype, tokens};
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
	has_args{false} {}


ASTMemberType::ASTMemberType(const Token &name,
                             TokenStream &tokens)
	:
	does_exist{true},
	has_args{false} {

	this->name = IDToken{name, tokens};

	// now there may follow type arguments, e.g. set(arg, key=val)
	auto token = tokens.next();

	// Check how many type arguments are required at minimum
	BasicType member_type = BasicType::from_type_token(this->name);
	auto num_expected_types = BasicType::expected_nested_types(member_type);

	if (token->type == token_type::LPAREN) {
		auto num_read_types = comma_list(
								token_type::RPAREN,
								tokens,
								num_expected_types,
								[this] (const Token &token, TokenStream &stream) {
									this->nested_types.emplace_back(token, stream);
								}
		);
		if (num_read_types < num_expected_types) {
			throw ASTError(
				std::string("expected at least ")
				+ std::to_string(num_expected_types)
				+ " arguments for "
				+ composite_type_to_string(member_type.composite_type)
				+ " declaration, but only "
				+ std::to_string(num_read_types)
				+ " could be found",
				*token, false
			);
		}

		if (this->args.size() > 0) {
			this->has_args = true;
		}
	}
	else if (num_expected_types > 0) {
		throw ASTError(
			std::string("expected at least ")
			+ std::to_string(num_expected_types)
			+ " arguments for "
			+ composite_type_to_string(member_type.composite_type)
			+ " declaration",
			*token, false
		);
	}
	else {
		tokens.reinsert_last();
	}
}


bool ASTMemberType::exists() const {
	return this->does_exist;
}


ASTMemberTypeArgument::ASTMemberTypeArgument(TokenStream &tokens)
	:
	has_key{false} {
	auto token = tokens.next();
	if (token->type != token_type::ID) {
		throw ASTError("expected argument value or key, but got", *token);
	}

	auto next_token = tokens.next();
	// check if the argument is keyed
	if (next_token->type == token_type::OPERATOR) {
		if (unlikely(op_from_token(*next_token) != nyan_op::ASSIGN)) {
			throw ASTError("expected argument keyed assignment, but got", *token);
		}

		this->has_key = true;
		this->key = IDToken(*token, tokens);

		token = tokens.next();
		if (unlikely(token->type != token_type::ID)) {
			throw ASTError("expected argument value, but got", *token);
		}
	} else {
		tokens.reinsert_last();
	}

	this->value = IDToken{*token, tokens};
}


ASTMemberValue::ASTMemberValue()
	:
	does_exist{false} {}


ASTMemberValue::ASTMemberValue(const IDToken &value)
	:
	does_exist{true},
	composite_type{composite_t::NONE} {

	this->values.emplace_back(value);
}


ASTMemberValue::ASTMemberValue(composite_t type,
                               TokenStream &tokens)
	:
	does_exist{true},
	composite_type{type} {

	token_type end_token;

	switch (this->composite_type) {
	case composite_t::SET:
	case composite_t::ORDEREDSET: {
		end_token = token_type::RBRACE;

		comma_list(
			end_token,
			tokens,
			[this] (const Token &token, TokenStream &stream) {
				const IDToken id_token = IDToken(token, stream);
				this->values.emplace_back(id_token);
			}
		);
	} break;
	case composite_t::DICT: {
		end_token = token_type::RBRACE;

		comma_list(
			end_token,
			tokens,
			[this] (const Token &token, TokenStream &stream) {
				std::vector<IDToken> id_tokens;

				// key
				id_tokens.emplace_back(token, stream);

				auto next_token = stream.next();
				if (next_token->type == token_type::COLON) {
					next_token = stream.next();
				}
				else {
					throw ASTError{"expected colon, but got", *next_token};
				}

				// value
				id_tokens.emplace_back(*next_token, stream);

				this->values.emplace_back(composite_type, id_tokens);
			}
		);
	} break;

	default:
		throw InternalError{"unknown container value type"};
	}


}


bool ASTMemberValue::exists() const {
	// the size of this->values doesn't matter, as the value could be an empty set.
	return this->does_exist;
}


const std::vector<ValueToken> &ASTMemberValue::get_values() const {
	return this->values;
}


const composite_t &ASTMemberValue::get_composite_type() const {
	return this->composite_type;
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

void ASTArgument::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	builder << "!" << this->arg.str();
	for (auto &param : this->params) {
		builder << " " << param.str();
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

	if (this->has_args) {
		builder << "(";
		for (auto &arg : this->args) {
			arg.strb(builder);
		}
		builder << ")";
	}
}


void ASTMemberTypeArgument::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	if (this->has_key) {
		builder << this->key.str() << "=";
	}

	builder << this->value.str();
}

void ASTMemberValue::strb(std::ostringstream &builder, int /*indentlevel*/) const {
	switch (this->composite_type) {
	case composite_t::NONE:
		builder << this->values[0].str();
		return;

	case composite_t::SET:
	case composite_t::DICT:
		builder << "{"; break;

	case composite_t::ORDEREDSET:
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

	switch (this->composite_type) {
	case composite_t::SET:
	case composite_t::ORDEREDSET:
	case composite_t::DICT:
		builder << "}"; break;

	default:
		throw InternalError{"unhandled container type"};
	}
}




ASTError::ASTError(const std::string &msg,
                   const Token &token,
                   bool add_token)
	:
	LangError{Location{token}, ""} {

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
	LangError{Location{token}, ""} {

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
