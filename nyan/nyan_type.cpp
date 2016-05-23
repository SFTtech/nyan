// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_type.h"

#include "nyan_ast.h"
#include "nyan_error.h"
#include "nyan_store.h"
#include "nyan_token.h"

namespace nyan {


TypeError::TypeError(const NyanLocation &location, const std::string &msg)
	:
	NyanFileError{location, msg} {}


nyan_type type_from_token(const NyanToken &tok) {
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


NyanType::NyanType(const NyanASTMemberType &ast_type,
                   const NyanStore &store)
	:
	value_type{nullptr},
	target{nullptr} {

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

	// primitive type like int or text
	auto it0 = primitive_types.find(ast_type.name.get());
	if (it0 != std::end(primitive_types)) {
		if (ast_type.has_payload) {
			throw ASTError{
				"primitive type can't have a type payload",
				ast_type.payload, false
			};
		}
		this->type = it0->second;
		this->container_type = nyan_container_type::SINGLE;
		return;
	}

	// container type like set(something)
	auto it1 = container_types.find(ast_type.name.get());
	if (it1 != std::end(container_types)) {
		if (not ast_type.has_payload) {
			throw ASTError{
				"container value type not specified",
				ast_type.name, false
			};
		}

		this->type = nyan_type::CONTAINER;
		this->container_type = it1->second;
		this->value_type = std::make_unique<NyanType>(
			ast_type.payload,
			store
		);
	}

	// type is not builtin, but has a payload
	if (ast_type.has_payload) {
		throw ASTError{
			"you can't assign a target to a nyanobject",
			ast_type.payload, false
		};
	}

	// look up the object and use it as type.
	NyanObject *type_obj = store.get(ast_type.name.get());
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
	value_type{nullptr},
	target{target} {}


NyanType::NyanType(nyan_container_type container_type,
                   std::unique_ptr<NyanType> &&value_type)
	:
	type{nyan_type::CONTAINER},
	container_type{container_type},
	value_type{std::move(value_type)},
	target{nullptr} {}


/* type payload parsing */
NyanType::NyanType(const NyanToken &token,
                   const NyanStore &store)
	:
	type{type_from_token(token)},
	container_type{nyan_container_type::SINGLE},
	value_type{nullptr} {

	switch (this->type) {
	case nyan_type::OBJECT:
		this->target = store.get(token.get());
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
	value_type{std::move(other.value_type)},
	target{other.target} {}


NyanType &NyanType::operator =(NyanType &&other) {
	this->type = other.type;
	this->container_type = other.container_type;
	this->value_type = std::move(other.value_type);
	this->target = other.target;
	return *this;
}


bool NyanType::is_primitive() const {
	switch (this->type) {
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
			if (this->value_type.get() == nullptr or
			    other.value_type.get() == nullptr) {
				throw NyanInternalError{"container type without value type"};
			}

			return other.value_type->is_child_of(*this->value_type);

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
	if (not other.is_container()) {
		// this also guarantees that other.value_type exists.
		return false;
	}

	return this->is_child_of(*other.value_type);
}


nyan_container_type NyanType::get_container_type() const {
	return this->container_type;
}


nyan_type NyanType::get_type() const {
	return this->type;
}

} // namespace nyan
