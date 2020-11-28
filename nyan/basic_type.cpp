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
	case primitive_t::OBJECT:
	case primitive_t::CONTAINER:
	case primitive_t::MODIFIER:
		return false;
	}

	throw InternalError{"unknown primitive type"};
}


bool BasicType::is_composite() const {
	return (this->composite_type != composite_t::NONE);
}


bool BasicType::is_container() const {
	switch (this->composite_type) {
	case composite_t::SET:
	case composite_t::ORDEREDSET:
	case composite_t::DICT:
		return true;
	case composite_t::NONE:
	case composite_t::ABSTRACT:
	case composite_t::CHILDREN:
	case composite_t::OPTIONAL:
		return false;
	}

	throw InternalError{"unknown composite type"};
}


bool BasicType::is_modifier() const {
	switch (this->composite_type) {
	case composite_t::ABSTRACT:
	case composite_t::CHILDREN:
	case composite_t::OPTIONAL:
		return true;
	case composite_t::NONE:
	case composite_t::SET:
	case composite_t::ORDEREDSET:
	case composite_t::DICT:
		return false;
	}

	throw InternalError{"unknown composite type"};
}


bool BasicType::operator ==(const BasicType &other) const {
	return (this->primitive_type == other.primitive_type and
	        this->composite_type == other.composite_type);
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
	static const std::unordered_map<std::string, composite_t> container_types = {
		{"set", composite_t::SET},
		{"orderedset", composite_t::ORDEREDSET},
		{"dict", composite_t::DICT}
	};

	// modifier type name map
	static const std::unordered_map<std::string, composite_t> modifier_types = {
		{"abstract", composite_t::ABSTRACT},
		{"children", composite_t::CHILDREN},
		{"optional", composite_t::OPTIONAL}
	};

	primitive_t type = primitive_t::OBJECT;
	composite_t composite_type = composite_t::NONE;

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
			composite_type = it1->second;
			break;
		}

		auto it2 = modifier_types.find(tok.get_first());
		if (it2 != std::end(modifier_types)) {
			type = primitive_t::MODIFIER;
			composite_type = it2->second;
		}
		break;
	}
	default:
		throw ASTError{"expected some type name but there is", tok};
	}

	return BasicType{type, composite_type};
}

} // namespace nyan
