// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

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
	case primitive_t::BOOLEAN:
	case primitive_t::TEXT:
	case primitive_t::FILENAME:
	case primitive_t::INT:
	case primitive_t::FLOAT:
		return true;
	case primitive_t::CONTAINER:
	case primitive_t::OBJECT:
		return false;
	}

	throw InternalError{"unknown primitive type"};
}


bool BasicType::is_container() const {
	return (this->container_type != container_t::SINGLE);
}


bool BasicType::operator ==(const BasicType &other) const {
	return (this->primitive_type == other.primitive_type and
	        this->container_type == other.container_type);
}


// textual type conversion for the type definition in a member
BasicType BasicType::from_type_token(const IDToken &tok) {
	// primitive type name map
	static const std::unordered_map<std::string, primitive_t> primitive_types = {
		{"bool", primitive_t::BOOLEAN},
		{"text", primitive_t::TEXT},
		{"file", primitive_t::FILENAME},
		{"int", primitive_t::INT},
		{"float", primitive_t::FLOAT}
	};

	// container type name map
	static const std::unordered_map<std::string, container_t> container_types = {
		{"set", container_t::SET},
		{"orderedset", container_t::ORDEREDSET},
		{"dict", container_t::DICT}
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
