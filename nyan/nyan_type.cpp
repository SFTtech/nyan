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


bool nyan_basic_type::is_fundamental() const {
	switch (this->primitive_type) {
	case nyan_primitive_type::TEXT:
	case nyan_primitive_type::FILENAME:
	case nyan_primitive_type::INT:
	case nyan_primitive_type::FLOAT:
		return true;
	case nyan_primitive_type::CONTAINER:
	case nyan_primitive_type::OBJECT:
		return false;
	}
}


bool nyan_basic_type::is_container() const {
	return (this->container_type != nyan_container_type::SINGLE);
}


bool nyan_basic_type::operator ==(const nyan_basic_type &other) const {
	return (this->primitive_type == other.primitive_type and
	        this->container_type == other.container_type);
}

nyan_basic_type type_from_value_token(const NyanToken &tok) {
	nyan_primitive_type value_type;

	switch (tok.type) {
	case token_type::ID:
		value_type = nyan_primitive_type::OBJECT;
		break;
	case token_type::INT:
		value_type = nyan_primitive_type::INT;
		break;
	case token_type::FLOAT:
		value_type = nyan_primitive_type::FLOAT;
		break;
	case token_type::STRING:
		value_type = nyan_primitive_type::TEXT;
		break;
	default:
		throw ASTError{"expected some value but there is", tok};
	}
	return {value_type, nyan_container_type::SINGLE};
}


nyan_basic_type type_from_type_token(const NyanToken &tok) {
	// primitive type name map
	static const std::unordered_map<std::string, nyan_primitive_type> primitive_types = {
		{"text", nyan_primitive_type::TEXT},
		{"file", nyan_primitive_type::FILENAME},
		{"int", nyan_primitive_type::INT},
		{"float", nyan_primitive_type::FLOAT}
	};

	// container type name map
	static const std::unordered_map<std::string, nyan_container_type> container_types = {
		{"set", nyan_container_type::SET},
		{"orderedset", nyan_container_type::ORDEREDSET}
	};


	nyan_primitive_type type = nyan_primitive_type::OBJECT;
	nyan_container_type container_type = nyan_container_type::SINGLE;

	switch (tok.type) {
	case token_type::ID: {
		auto it0 = primitive_types.find(tok.get());
		if (it0 != std::end(primitive_types)) {
			type = it0->second;
		}

		auto it1 = container_types.find(tok.get());
		if (it1 != std::end(container_types)) {
			type = nyan_primitive_type::CONTAINER;
			container_type = it1->second;
		}
		break;
	}
	default:
		throw ASTError("expected some type name but there is", tok);
	}

	return {type, container_type};
}


NyanType::NyanType(nyan_primitive_type type)
	:
	basic_type{type, nyan_container_type::SINGLE} {

	if (not this->is_fundamental()) {
		throw NyanInternalError{
			"initialized NyanType primitive constructor with non-fundamental type"
		};
	}
}


NyanType::NyanType(const NyanASTMemberType &ast_type,
                   const NyanDatabase &database)
	:
	element_type{nullptr},
	target{nullptr} {

	this->basic_type = type_from_type_token(ast_type.name);

	// test if the type is primitive (int, float, text, ...)
	if (this->basic_type.is_fundamental()) {
		if (ast_type.has_payload) {
			throw ASTError{
				"fundamental type can't have a type payload",
				ast_type.payload, false
			};
		}
		return;
	}

	// container type like set(something)
	if (this->basic_type.is_container()) {
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

	this->basic_type = {
		nyan_primitive_type::OBJECT,
		nyan_container_type::SINGLE
	};

	this->target = type_obj;
}


NyanType::NyanType(NyanObject *target)
	:
	basic_type{nyan_primitive_type::OBJECT, nyan_container_type::SINGLE},
	element_type{nullptr},
	target{target} {}


NyanType::NyanType(nyan_container_type container_type,
                   std::unique_ptr<NyanType> &&element_type)
	:
	basic_type{nyan_primitive_type::CONTAINER, container_type},
	element_type{std::move(element_type)},
	target{nullptr} {}


/* create a nyan_primitive_type from some token, used e.g. for type payload parsing */
NyanType::NyanType(const NyanToken &token,
                   const NyanDatabase &database,
                   bool is_type_decl)
	:
	element_type{nullptr} {

	if (is_type_decl) {
		this->basic_type = type_from_type_token(token);
	}
	else {
		this->basic_type = type_from_value_token(token);
	}

	switch (this->get_primitive_type()) {
	case nyan_primitive_type::OBJECT:
		this->target = database.get(token.get());
		if (this->target == nullptr) {
			throw TypeError{
				NyanLocation{token},
				"unknown object name"
			};
		}
		break;

	case nyan_primitive_type::INT:
	case nyan_primitive_type::FLOAT:
	case nyan_primitive_type::TEXT:
		// no target needs to be saved
		break;

	default:
		throw NyanInternalError{"unhandled type from token"};
	}
}


NyanType::NyanType(NyanType &&other)
	:
	basic_type{std::move(other.basic_type)},
	element_type{std::move(other.element_type)},
	target{other.target} {}


NyanType &NyanType::operator =(NyanType &&other) {
	this->basic_type = std::move(other.basic_type);
	this->element_type = std::move(other.element_type);
	this->target = other.target;
	return *this;
}


bool NyanType::is_fundamental() const {
	return this->basic_type.is_fundamental();
}


bool NyanType::is_container() const {
	return this->basic_type.is_container();
}


bool NyanType::is_container(nyan_container_type type) const {
	return this->get_container_type() == type;
}


bool NyanType::is_child_of(const NyanType &other) const {
	// two non-fundamental types need special handling
	if (not this->is_fundamental() and
	    not other.is_fundamental()) {

		if (this->get_primitive_type() != other.get_primitive_type()) {
			return false;
		}

		switch (this->get_primitive_type()) {
		case nyan_primitive_type::OBJECT:
			if (other.target == nullptr) {
				// if the target is nullptr, any object is allowed!
				return true;
			}

			return this->target->is_child_of(other.target);

		case nyan_primitive_type::CONTAINER:
			if (this->element_type.get() == nullptr or
			    other.element_type.get() == nullptr) {
				throw NyanInternalError{"container type without value type"};
			}

			return other.element_type->is_child_of(*this->element_type);

		default:
			throw NyanInternalError{"invalid non-primitive type encountered"};
		}
	}
	// if at least one of them is fundamental:
	else if (this->get_primitive_type() == other.get_primitive_type()) {
		return true;
	}
	else {
		return false;
	}
}


bool NyanType::is_child_of(const NyanObject *obj) const {
	if (this->get_primitive_type() != nyan_primitive_type::OBJECT) {
		return false;
	}

	if (obj == nullptr) {
		// any object is allowed, so we are a child.
		return true;
	}

	return this->target->is_child_of(obj);
}


bool NyanType::is_parent_of(const NyanObject *obj) const {
	if (this->get_primitive_type() != nyan_primitive_type::OBJECT) {
		return false;
	}

	if (this->target == nullptr) {
		// we allow any object, so we are a parent of obj.
		return true;
	}
	else if (obj == nullptr) {
		// we don't allow any objet, so we can't be the parent.
		return false;
	}

	return obj->is_child_of(this->target);
}


bool NyanType::is_basic_compatible(const nyan_basic_type &type) const {
	return (this->basic_type == type);
}


bool NyanType::can_be_in(const NyanType &other) const {
	// this check also guarantees that other.element_type exists:
	if (not other.is_container()) {
		return false;
	}

	return this->is_child_of(*other.element_type);
}


const nyan_basic_type &NyanType::get_basic_type() const {
	return this->basic_type;
}


const nyan_container_type &NyanType::get_container_type() const {
	return this->basic_type.container_type;
}


const nyan_primitive_type &NyanType::get_primitive_type() const {
	return this->basic_type.primitive_type;
}


const NyanType *NyanType::get_element_type() const {
	return this->element_type.get();
}


std::string NyanType::str() const {
	if (this->is_fundamental()) {
		return type_to_string(this->get_primitive_type());
	}
	else {
		if (this->get_primitive_type() == nyan_primitive_type::OBJECT) {
			if (this->target == nullptr) {
				return "__any__";
			}
			else {
				return this->target->repr();
			}
		}

		if (this->get_container_type() == nyan_container_type::SINGLE) {
			throw NyanInternalError{
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
