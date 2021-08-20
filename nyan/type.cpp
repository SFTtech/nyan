// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "type.h"

#include "ast.h"
#include "compiler.h"
#include "error.h"
#include "id_token.h"
#include "meta_info.h"
#include "namespace.h"
#include "namespace_finder.h"
#include "object_state.h"
#include "state.h"
#include "token.h"


namespace nyan {


Type::Type(const ASTMemberType &root_ast_type,
           const NamespaceFinder &scope,
           const Namespace &ns,
           const MetaInfo &type_info)
	:
	element_type{std::nullopt} {

	// ast -> type conversion works like this:
	// we fold all modifiers into binary flags, and intialize this type
	// with the first non-modifier basic type.
	// if the basic type is a composite (i.e. requires further nested types like the
	// set element type),
	// recursively create more types with this same constructor.
	//
	// set(int) -> set with element_type [int]
	// optional(int) -> int with optional flag
	// optional(set(int)) -> set with optional flag with element_type [int]
	// set(optional(int)) -> error, element_type can't be optional
	// dict(optional(int), ...) -> error, key_type can't be optional
	// dict(int, optional(int)) -> dict with element_type [key=int, value=int with optional flag]
	// optional(dict(abstract(Obj1), optional(abstract(Obj2))))
	//  -> dict with optional flag, element type [object=Obj1 with abstract flag, object=Obj2 with abstract and optional flag]

	const ASTMemberType *ast_type = &root_ast_type;
	BasicType basic_type = BasicType::from_type_token(ast_type->name);

	// validation method for the nested type count
	auto expect_nested_types = [] (const BasicType &basic_type,
	                               const ASTMemberType &ast_type) -> size_t {
		size_t count = basic_type.expected_nested_types();
		if (not (count == ast_type.nested_types.size())) {
			throw ASTError{
				std::string("only ")
				+ std::to_string(ast_type.nested_types.size())
				+ " container element types specified, expected "
				+ std::to_string(count),
				ast_type.name, false
			};
		}
		return count;
	};


	// consume all modifiers and convert them to flags:
	// optional(int) becomes int with flag optional.

	while (basic_type.is_modifier()) {
		expect_nested_types(basic_type, *ast_type);

		// TODO: maybe unify BasicType.composite_type and modifier_t
		//       then we don't need this mapping.
		switch (basic_type.composite_type) {
		case composite_t::ABSTRACT:
			this->modifiers.set(modifier_t::ABSTRACT);
			break;
		case composite_t::OPTIONAL:
			this->modifiers.set(modifier_t::OPTIONAL);
			break;
		case composite_t::CHILDREN:
			this->modifiers.set(modifier_t::CHILDREN);
			break;
		default:
			throw InternalError{"unhandled modifier type"};
		}

		if (unlikely(basic_type.expected_nested_types() != 1)) {
			throw InternalError{"modifier doesn't expect 1 nested type"};
		}

		basic_type = BasicType::from_type_token(ast_type->nested_types[0].name);
		ast_type = &ast_type->nested_types[0];
	}

	// now we've processed all modifier type wrappers
	this->basic_type = std::move(basic_type);

	// test if the type is primitive (int, float, text, ...)
	// then type construction is already done.
	if (basic_type.is_fundamental()) {
		if (unlikely(ast_type->nested_types.size() != 0)) {
			throw InternalError{"ast generated > 0 nested types for fundamental type"};
		}
		return;
	}
	else if (basic_type.is_composite()) {

		size_t expected_element_types = expect_nested_types(basic_type, *ast_type);

		std::vector<Type> nested_types{};
		nested_types.reserve(expected_element_types);

		for (size_t i = 0; i < expected_element_types; i++) {
			// calls the the same constructor recursively!
			nested_types.emplace_back(
				ast_type->nested_types.at(i),
				scope,
				ns,
				type_info
			);
		}

		switch (basic_type.composite_type) {
		case composite_t::DICT:
		case composite_t::SET:
		case composite_t::ORDEREDSET:
			// dict key type or set value types can't be optional
			if (nested_types[0].has_modifier(modifier_t::OPTIONAL)) {
				throw TypeError{
					ast_type->nested_types[0].name.get_start_location(),
					"container key type can't be optional"
				};
			}
			break;
		default:
			throw InternalError{"unhandled composite type"};
		}

		this->element_type = std::move(nested_types);

		return;
	}
	else if (basic_type.is_object()) {
		this->obj_ref = scope.find(ns, ast_type->name, type_info);
	}
	else {
		throw InternalError{"unhandled BasicType to Type conversion"};
	}
}


bool Type::is_object() const {
	return this->basic_type.is_object();
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


bool Type::is_hashable() const {
	if (this->is_fundamental()) {
		return true;
	}
	else if (this->basic_type.is_object()) {
		return true;
	}

	// containers are non-hashable
	return false;
}


bool Type::is_basic_type_match(const BasicType &type) const {
	return (this->basic_type == type);
}


bool Type::has_modifier(modifier_t mod) const {
	return this->modifiers[mod] == true;
}


const fqon_t &Type::get_fqon() const {
	return this->obj_ref;
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


const std::vector<Type> &Type::get_element_type() const {
	if (unlikely(not this->element_type.has_value())) {
		throw InternalError{"container element type is nonexisting"};
	}
	return this->element_type.value();
}


std::string Type::str() const {
	if (this->is_fundamental()) {
		return type_to_string(this->get_primitive_type());
	}
	else {
		if (this->get_primitive_type() == primitive_t::OBJECT) {
			return this->obj_ref;
		}

		if (unlikely(this->get_composite_type() == composite_t::SINGLE)) {
			throw InternalError{
				"single value encountered when expecting composite"
			};
		}

		std::ostringstream builder;

		builder << composite_type_to_string(this->get_composite_type())
		        << "(";

		util::strjoin(
			builder, ", ", this->get_element_type(),
			[](auto &builder, auto& item) {
				builder << item.str();
			}
		);

		builder << ")";

		return builder.str();
	}
}

bool Type::operator ==(const Type &other) const {
	if (not this->is_basic_type_match(other.get_basic_type())) {
		return false;
	}

	if (this->get_element_type() != other.get_element_type()) {
		return false;
	}

	if (this->is_object()) {
		// For objects the fqons must be equal
		if (this->get_fqon() != other.get_fqon()) {
			return false;
		}
	}

	return true;
}

} // namespace nyan
