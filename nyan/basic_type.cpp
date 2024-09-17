// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

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
	case primitive_t::NONE:
		return true;
	case primitive_t::OBJECT:
	case primitive_t::CONTAINER:
	case primitive_t::MODIFIER:
		return false;
	}

	throw InternalError{"unknown primitive type"};
}


bool BasicType::is_composite() const {
	return (this->composite_type != composite_t::SINGLE);
}


bool BasicType::is_container() const {
	switch (this->composite_type) {
	case composite_t::SET:
	case composite_t::ORDEREDSET:
	case composite_t::DICT:
		return true;
	case composite_t::SINGLE:
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
	case composite_t::SINGLE:
	case composite_t::SET:
	case composite_t::ORDEREDSET:
	case composite_t::DICT:
		return false;
	}

	throw InternalError{"unknown composite type"};
}


size_t BasicType::expected_nested_types() const {
	if (this->is_fundamental()) {
		return 0;
	}

	switch (this->composite_type) {
		// containers
	case composite_t::SET:
	case composite_t::ORDEREDSET:
		return 1;
	case composite_t::DICT:
		return 2;

		// modifiers
	case composite_t::ABSTRACT:
	case composite_t::CHILDREN:
	case composite_t::OPTIONAL:
		return 1;

		// else, primitive value
	case composite_t::SINGLE:
		return 0;

	default:
		throw Error{"unhandled composite type"};
	}
}


bool BasicType::operator==(const BasicType &other) const {
	return (this->primitive_type == other.primitive_type and this->composite_type == other.composite_type);
}


std::string BasicType::str() const {
	if (this->is_fundamental()) {
		return type_to_string(this->primitive_type);
	}
	else if (this->is_composite()) {
		return composite_type_to_string(this->composite_type);
	}

	throw InternalError{"basic string neither fundamental nor composite"};
}


// textual type conversion for the type definition in a member
BasicType BasicType::from_type_token(const IDToken &tok) {
	// TODO: replace those lookup maps with a constexpr compiletime map.

	// primitive type name map
	static const std::unordered_map<std::string, primitive_t> primitive_types = {
		{"bool", primitive_t::BOOLEAN},
		{"text", primitive_t::TEXT},
		{"file", primitive_t::FILENAME},
		{"int", primitive_t::INT},
		{"float", primitive_t::FLOAT}};

	// container type name map
	static const std::unordered_map<std::string, composite_t> container_types = {
		{"set", composite_t::SET},
		{"orderedset", composite_t::ORDEREDSET},
		{"dict", composite_t::DICT}};

	// modifier type name map
	static const std::unordered_map<std::string, composite_t> modifiers = {
		{"abstract", composite_t::ABSTRACT},
		{"children", composite_t::CHILDREN},
		{"optional", composite_t::OPTIONAL}};

	primitive_t type = primitive_t::OBJECT;
	composite_t composite_type = composite_t::SINGLE;

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

		auto it2 = modifiers.find(tok.get_first());
		if (it2 != std::end(modifiers)) {
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
