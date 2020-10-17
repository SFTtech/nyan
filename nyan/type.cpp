// Copyright 2016-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "type.h"

#include "ast.h"
#include "error.h"
#include "id_token.h"
#include "meta_info.h"
#include "namespace.h"
#include "namespace_finder.h"
#include "object_state.h"
#include "state.h"
#include "token.h"


namespace nyan {


Type::Type(const ASTMemberType &ast_type,
           const NamespaceFinder &scope,
           const Namespace &ns,
           const MetaInfo &type_info)
	:
	element_type{nullptr} {

	this->basic_type = BasicType::from_type_token(ast_type.name);

	// test if the type is primitive (int, float, text, ...)
	if (this->basic_type.is_fundamental()) {
		return;
	}

	// container type like set(something)
	if (this->basic_type.is_container()) {
		if (not ast_type.has_args) {
			throw ASTError{
				"container content type not specified",
				ast_type.name, false
			};
		}

		// TODO: Element type is more complex for dicts
		this->element_type = std::make_unique<Type>(
			ast_type.args.at(0).value, 
			scope,
			ns,
			type_info
		);
		return;
	}

	// here, type must be a OBJECT.

	this->basic_type = {
		primitive_t::OBJECT,
		container_t::SINGLE
	};

	this->target = scope.find(ns, ast_type.name, type_info);
}


/*
 * create a primitive_t from some token,
 * used e.g. for type payload parsing
 */
Type::Type(const IDToken &token,
           const NamespaceFinder &scope,
           const Namespace &ns,
           const MetaInfo &type_info)
	:
	element_type{nullptr} {

	this->basic_type = BasicType::from_type_token(token);

	switch (this->get_primitive_type()) {
	case primitive_t::OBJECT:
		this->target = scope.find(ns, token, type_info);
		break;

	case primitive_t::INT:
	case primitive_t::FLOAT:
	case primitive_t::TEXT:
		// no target needs to be saved
		break;

	default:
		throw InternalError{"unhandled type from token"};
	}
}


bool Type::is_fundamental() const {
	return this->basic_type.is_fundamental();
}


bool Type::is_container() const {
	return this->basic_type.is_container();
}


bool Type::is_container(container_t type) const {
	return this->get_container_type() == type;
}


bool Type::is_basic_type_match(const BasicType &type) const {
	return (this->basic_type == type);
}


const fqon_t &Type::get_target() const {
	return this->target;
}


const BasicType &Type::get_basic_type() const {
	return this->basic_type;
}


const container_t &Type::get_container_type() const {
	return this->basic_type.container_type;
}


const primitive_t &Type::get_primitive_type() const {
	return this->basic_type.primitive_type;
}


const Type *Type::get_element_type() const {
	return this->element_type.get();
}


std::string Type::str() const {
	if (this->is_fundamental()) {
		return type_to_string(this->get_primitive_type());
	}
	else {
		if (this->get_primitive_type() == primitive_t::OBJECT) {
			return this->target;
		}

		if (this->get_container_type() == container_t::SINGLE) {
			throw InternalError{
				"single value encountered when expecting container"
			};
		}

		std::ostringstream builder;

		builder << container_type_to_string(this->get_container_type())
		        << "("
		        << this->element_type->str()
		        << ")";

		return builder.str();
	}
}

} // namespace nyan
