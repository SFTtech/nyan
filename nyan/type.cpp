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

	// composite type like set(something)
	if (this->basic_type.is_composite()) {
		auto expected_element_types = BasicType::expected_nested_types(basic_type);
		if (not (expected_element_types == ast_type.nested_types.size())) {
			throw ASTError{
				std::string("only ")
				+ std::to_string(ast_type.nested_types.size())
				+ " container element types specified, expected "
				+ std::to_string(expected_element_types),
				ast_type.name, false
			};
		}

		std::vector<Type> types;

		for (unsigned int i = 0; i < expected_element_types; i++) {
			types.emplace_back(
				ast_type.nested_types.at(i),
				scope,
				ns,
				type_info
			);
		}

		this->element_type = std::make_unique<std::vector<Type>>(types);

		return;
	}

	// here, type must be a OBJECT.

	this->basic_type = {
		primitive_t::OBJECT,
		composite_t::NONE
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
	case primitive_t::BOOLEAN:
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


bool Type::is_container(composite_t type) const {
	return (this->basic_type.is_container() and
			this->get_composite_type() == type);
}

bool Type::is_modifier() const {
	return this->basic_type.is_modifier();
}


bool Type::is_modifier(composite_t type) const {
	return (this->basic_type.is_modifier() and
			this->get_composite_type() == type);
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


const composite_t &Type::get_composite_type() const {
	return this->basic_type.composite_type;
}


const primitive_t &Type::get_primitive_type() const {
	return this->basic_type.primitive_type;
}


const std::vector<Type> *Type::get_element_type() const {
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

		if (this->get_composite_type() == composite_t::NONE) {
			throw InternalError{
				"single value encountered when expecting composite"
			};
		}

		std::ostringstream builder;

		builder << composite_type_to_string(this->get_composite_type())
		        << "(";

		for (auto &elem_type : *this->get_element_type()) {
			builder << elem_type.str();
		}

		builder << ")";

		return builder.str();
	}
}

} // namespace nyan
