// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "basic_type.h"

#include "ast.h"
#include "error.h"
#include "id_token.h"
#include "token.h"


namespace nyan {

bool BasicType::is_object() const {
	return (this->primitive_type == primitive_t::OBJECT);
}

bool BasicType::is_fundamental() const {
	switch (this->primitive_type) {
	case primitive_t::TEXT:
	case primitive_t::FILENAME:
	case primitive_t::INT:
	case primitive_t::FLOAT:
		return true;
	case primitive_t::CONTAINER:
	case primitive_t::OBJECT:
		return false;
	}
}


bool BasicType::is_container() const {
	return (this->container_type != container_t::SINGLE);
}


bool BasicType::operator ==(const BasicType &other) const {
	return (this->primitive_type == other.primitive_type and
	        this->container_type == other.container_type);
}


// type of a right hand side assignment
BasicType BasicType::from_value_token(const IDToken &tok) {
	primitive_t value_type;

	switch (tok.get_type()) {
	case token_type::ID:
		value_type = primitive_t::OBJECT;
		break;
	case token_type::INT:
		value_type = primitive_t::INT;
		break;
	case token_type::FLOAT:
		value_type = primitive_t::FLOAT;
		break;
	case token_type::STRING:
		value_type = primitive_t::TEXT;
		break;
	default:
		throw ASTError{"expected some value but there is", tok};
	}
	return BasicType{value_type, container_t::SINGLE};
}


// textual type conversion for the type definition in a member
BasicType BasicType::from_type_token(const IDToken &tok) {
	// primitive type name map
	static const std::unordered_map<std::string, primitive_t> primitive_types = {
		{"text", primitive_t::TEXT},
		{"file", primitive_t::FILENAME},
		{"int", primitive_t::INT},
		{"float", primitive_t::FLOAT}
	};

	// container type name map
	static const std::unordered_map<std::string, container_t> container_types = {
		{"set", container_t::SET},
		{"orderedset", container_t::ORDEREDSET}
	};


	primitive_t type = primitive_t::OBJECT;
	container_t container_type = container_t::SINGLE;

	switch (tok.get_type()) {
		// type names are always identifiers:
	case token_type::ID: {
		auto it0 = primitive_types.find(tok.get_first());
		if (it0 != std::end(primitive_types)) {
			type = it0->second;
			break;
		}

		auto it1 = container_types.find(tok.get_first());
		if (it1 != std::end(container_types)) {
			type = primitive_t::CONTAINER;
			container_type = it1->second;
		}
		break;
	}
	default:
		throw ASTError{"expected some type name but there is", tok};
	}

	return BasicType{type, container_type};
}


} // namespace nyan
