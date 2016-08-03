// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_type.h"

#include "nyan_ast.h"
#include "nyan_database.h"
#include "nyan_error.h"
#include "nyan_token.h"

namespace nyan {


TypeError::TypeError(const NyanLocation &location, const std::string &msg)
	:
	NyanFileError{location, msg} {}


bool type_is_primitive(nyan_type type) {
	switch (type) {
	case nyan_type::TEXT:
	case nyan_type::FILENAME:
	case nyan_type::INT:
	case nyan_type::FLOAT:
		return true;
	case nyan_type::CONTAINER:
	case nyan_type::OBJECT:
		return false;
	}
}


nyan_type type_from_value_token(const NyanToken &tok) {
	nyan_type value_type;

	switch (tok.type) {
	case token_type::ID:
		value_type = nyan_type::OBJECT;
		break;
	case token_type::INT:
		value_type = nyan_type::INT;
		break;
	case token_type::FLOAT:
		value_type = nyan_type::FLOAT;
		break;
	case token_type::STRING:
		value_type = nyan_type::TEXT;
		break;
	default:
		throw ASTError("expected some value but there is", tok);
	}
	return value_type;
}


std::pair<nyan_type, nyan_container_type>
type_from_type_token(const NyanToken &tok) {
	// primitive type name map
	static const std::unordered_map<std::string, nyan_type> primitive_types = {
		{"text", nyan_type::TEXT},
		{"file", nyan_type::FILENAME},
		{"int", nyan_type::INT},
		{"float", nyan_type::FLOAT}
	};

	// container type name map
	static const std::unordered_map<std::string, nyan_container_type> container_types = {
		{"set", nyan_container_type::SET},
		{"orderedset", nyan_container_type::ORDEREDSET}
	};


	nyan_type type = nyan_type::OBJECT;
	nyan_container_type container_type = nyan_container_type::SINGLE;

	switch (tok.type) {
	case token_type::ID: {
		auto it0 = primitive_types.find(tok.get());
		if (it0 != std::end(primitive_types)) {
			type = it0->second;
		}

		auto it1 = container_types.find(tok.get());
		if (it1 != std::end(container_types)) {
			type = nyan_type::CONTAINER;
			container_type = it1->second;
		}
		break;
	}
	default:
		throw ASTError("expected some type name but there is", tok);
	}

	return std::make_pair(type, container_type);
}


NyanType::NyanType(nyan_type type)
	:
	type{type},
	container_type{nyan_container_type::SINGLE} {

	if (not this->is_primitive()) {
		throw NyanInternalError{
			"initialized NyanType constructor with non-primitive type"
		};
	}
}


NyanType::NyanType(const NyanASTMemberType &ast_type,
                   const NyanDatabase &database)
	:
	element_type{nullptr},
	target{nullptr} {

	auto parsed_type = type_from_type_token(ast_type.name);
	this->type = std::get<0>(parsed_type);
	this->container_type = std::get<1>(parsed_type);

	// test if the type is primitive (int, float, text, ...)
	if (type_is_primitive(this->type)) {
		if (ast_type.has_payload) {
			throw ASTError{
				"primitive type can't have a type payload",
				ast_type.payload, false
			};
		}
		return;
	}

	// container type like set(something)
	if (this->type == nyan_type::CONTAINER) {
		if (not ast_type.has_payload) {
			throw ASTError{
				"container value type not specified",
				ast_type.name, false
			};
		}

		this->element_type = std::make_unique<NyanType>(
			ast_type.payload,
			database,
			true
		);
		return;
	}

	// here, type must be a OBJECT.

	// type is not builtin, but has a payload
	if (ast_type.has_payload) {
		throw ASTError{
			"you can't assign a target to a nyanobject",
			ast_type.payload, false
		};
	}

	// look up the object and use it as type.
	NyanObject *type_obj = database.get(ast_type.name.get());
	if (type_obj == nullptr) {
		throw ASTError{"unknown NyanObject used as type", ast_type.name};
	}

	this->type = nyan_type::OBJECT;
	this->container_type = nyan_container_type::SINGLE;
	this->target = type_obj;
}


NyanType::NyanType(NyanObject *target)
	:
	type{nyan_type::OBJECT},
	container_type{nyan_container_type::SINGLE},
	element_type{nullptr},
	target{target} {}


NyanType::NyanType(nyan_container_type container_type,
                   std::unique_ptr<NyanType> &&element_type)
	:
	type{nyan_type::CONTAINER},
	container_type{container_type},
	element_type{std::move(element_type)},
	target{nullptr} {}


/* create a nyan_type from some token, used e.g. for type payload parsing */
NyanType::NyanType(const NyanToken &token,
                   const NyanDatabase &database,
                   bool is_type_decl)
	:
	container_type{nyan_container_type::SINGLE},
	element_type{nullptr} {

	if (is_type_decl) {
		auto parsed_type = type_from_type_token(token);
		this->type = std::get<0>(parsed_type);
		this->container_type = std::get<1>(parsed_type);
	}
	else {
		this->type = type_from_value_token(token);
	}

	switch (this->type) {
	case nyan_type::OBJECT:
		this->target = database.get(token.get());
		if (this->target == nullptr) {
			throw TypeError{
				NyanLocation{token},
				"unknown object name"
			};
		}
		break;

	case nyan_type::INT:
	case nyan_type::FLOAT:
	case nyan_type::TEXT:
		// no target needs to be saved
		break;

	default:
		throw NyanInternalError{"unhandled type from token"};
	}
}


NyanType::NyanType(NyanType &&other)
	:
	type{other.type},
	container_type{other.container_type},
	element_type{std::move(other.element_type)},
	target{other.target} {}


NyanType &NyanType::operator =(NyanType &&other) {
	this->type = other.type;
	this->container_type = other.container_type;
	this->element_type = std::move(other.element_type);
	this->target = other.target;
	return *this;
}


bool NyanType::is_primitive() const {
	return type_is_primitive(this->type);
}


bool NyanType::is_container() const {
	return (this->container_type != nyan_container_type::SINGLE);
}


bool NyanType::is_container(nyan_container_type type) const {
	return this->get_container_type() == type;
}


bool NyanType::is_child_of(const NyanType &other) const {
	// two non-primitive types need special handling
	if (not this->is_primitive() and
	    not other.is_primitive()) {

		if (this->type != other.type) {
			return false;
		}

		switch (this->type) {
		case nyan_type::OBJECT:
			if (other.target == nullptr) {
				// if the target is nullptr, any object is allowed!
				return true;
			}

			return this->target->is_child_of(other.target);

		case nyan_type::CONTAINER:
			if (this->element_type.get() == nullptr or
			    other.element_type.get() == nullptr) {
				throw NyanInternalError{"container type without value type"};
			}

			return other.element_type->is_child_of(*this->element_type);

		default:
			throw NyanInternalError{"invalid non-primitive type encountered"};
		}
	}
	else if (this->type == other.type) {
		return true;
	}
	else {
		return false;
	}
}


bool NyanType::can_be_in(const NyanType &other) const {
	// this check also guarantees that other.element_type exists:
	if (not other.is_container()) {
		return false;
	}

	return this->is_child_of(*other.element_type);
}


nyan_container_type NyanType::get_container_type() const {
	return this->container_type;
}


nyan_type NyanType::get_type() const {
	return this->type;
}


std::string NyanType::str() const {
	if (this->is_primitive()) {
		return type_to_string(this->get_type());
	}
	else {
		if (this->type == nyan_type::OBJECT) {
			if (this->target == nullptr) {
				return "__any__";
			}
			else {
				return this->target->repr();
			}
		}

		if (this->container_type == nyan_container_type::SINGLE) {
			throw NyanInternalError{
				"single value encountered when expecting container"
			};
		}

		std::ostringstream builder;

		builder << container_type_to_string(this->container_type)
		        << "("
		        << this->element_type->str()
		        << ")";

		return builder.str();
	}
}

} // namespace nyan
