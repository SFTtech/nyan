// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value.h"

#include "boolean.h"
#include "dict.h"
#include "file.h"
#include "number.h"
#include "object.h"
#include "orderedset.h"
#include "set.h"
#include "text.h"
#include "../ast.h"
#include "../error.h"
#include "../member.h"
#include "../token.h"


namespace nyan {


static ValueHolder value_from_id_token(const Type &target_type,
                                          const IDToken &id_token,
                                          const std::function<fqon_t(const Type &, const IDToken &)> &get_obj_value) {

	switch (target_type.get_primitive_type()) {
	case primitive_t::BOOLEAN:
		return {std::make_shared<Boolean>(id_token)};

	case primitive_t::TEXT:
		return {std::make_shared<Text>(id_token)};

	case primitive_t::INT: {
		if (id_token.get_type() == token_type::INF) {
			return {std::make_shared<Int>(id_token)};
		}
		else if (id_token.get_type() == token_type::INT) {
			return {std::make_shared<Int>(id_token)};
		}
		else if (id_token.get_type() == token_type::FLOAT) {
			return {std::make_shared<Float>(id_token)};
		}
		throw LangError{
			id_token,
			"invalid token for int, expected int or inf"
		};
	}
	case primitive_t::FLOAT: {
		if (id_token.get_type() == token_type::INF) {
			return {std::make_shared<Float>(id_token)};
		}
		else if (id_token.get_type() == token_type::INT) {
			return {std::make_shared<Int>(id_token)};
		}
		else if (id_token.get_type() == token_type::FLOAT) {
			return {std::make_shared<Float>(id_token)};
		}
		throw LangError{
			id_token,
			"invalid token for float, expected float or inf"
		};
	}
	case primitive_t::FILENAME: {
		// TODO: make relative to current namespace
		return {std::make_shared<Filename>(id_token)};
	}
	case primitive_t::OBJECT: {

		if (unlikely(id_token.get_type() != token_type::ID)) {
			throw LangError{
				id_token,
				"invalid value for object, expecting object id"
			};
		}

		fqon_t obj_id = get_obj_value(target_type, id_token);

		return {std::make_shared<ObjectValue>(std::move(obj_id))};
	}
	default:
		throw InternalError{"non-implemented value type"};
	}

	return {nullptr};
}


static std::vector<ValueHolder> value_from_value_token(const std::vector<Type> &target_types,
                                          			   const ValueToken &value_token,
                                          			   const std::function<fqon_t(const Type &, const IDToken &)> &get_obj_value) {
	if (unlikely(target_types.size() != value_token.get_value().size())) {
		throw TypeError(
				value_token.get_start_location(),
				std::string("ValueToken has ")
				+ std::to_string(value_token.get_value().size())
				+ " elements, but only "
				+ std::to_string(target_types.size())
				+ " have been requested"
			);
	}

	std::vector<ValueHolder> values;
	values.reserve(value_token.get_value().size());

	for (unsigned int i = 0; i < value_token.get_value().size(); i++) {
		values.push_back(value_from_id_token(
				target_types.at(i),
				value_token.get_value().at(i),
				get_obj_value
			)
		);
	}

	return values;
}


ValueHolder Value::from_ast(const Type &target_type,
                            const ASTMemberValue &astmembervalue,
                            const std::function<fqon_t(const Type &, const IDToken &)> &get_obj_value) {
	if (not target_type.is_container()) {
		// don't allow more than one value for a single-value type
		if (astmembervalue.get_values().size() > 1) {
			throw TypeError{
				astmembervalue.get_values()[1].get_start_location(),
				"storing multiple values in non-container"
			};
		}

		std::vector<Type> target_types = {target_type};
		return value_from_value_token(target_types,
		                           	  astmembervalue.get_values()[0],
		                           	  get_obj_value)[0];
	}

	composite_t composite_type = astmembervalue.get_composite_type();

	// For sets/orderedsets (with primitive values)
	std::vector<ValueHolder> values;

	// For dicts (with key-value pairs)
	std::unordered_map<ValueHolder, ValueHolder> items;

	if (composite_type == composite_t::SET || composite_type == composite_t::ORDEREDSET) {
		// process multi-value values (orderedsets etc)
		values.reserve(astmembervalue.get_values().size());

		// convert all tokens to values
		const std::vector<Type> *element_type = target_type.get_element_type();
		if (unlikely(element_type == nullptr)) {
			throw InternalError{"container element type is nonexisting"};
		}

		for (auto &value_token : astmembervalue.get_values()) {
			values.push_back(
				value_from_value_token(*element_type,
									   value_token,
									   get_obj_value)[0]
			);
		}

		switch (composite_type) {
		case composite_t::SET:
			// create a set from the value list
			return {std::make_shared<Set>(std::move(values))};

		case composite_t::ORDEREDSET:
			return {std::make_shared<OrderedSet>(std::move(values))};

		default:
			throw InternalError{"value creation for unhandled container type"};
		}
	}
	else if (composite_type == composite_t::DICT) {
		items.reserve(astmembervalue.get_values().size());

		// convert all tokens to values
		const std::vector<Type> *element_type = target_type.get_element_type();
		if (unlikely(element_type == nullptr)) {
			throw InternalError{"container element type is nonexisting"};
		}

		for (auto &value_token : astmembervalue.get_values()) {
			std::vector<ValueHolder> keyval = value_from_value_token(
				*element_type,
				value_token,
				get_obj_value
			);

			items.insert(std::make_pair(keyval[0], keyval[1]));
		}

		return {std::make_shared<Dict>(std::move(items))};
	}
	else {
		throw InternalError{"value creation for unhandled container type"};
	}
}


void Value::apply(const Member &change) {
	// extract the member's value,
	// this is just the data of the member,
	// no parent data is included.
	const Value &value = change.get_value();

	// TODO: cache usage: if the value has a cached value,
	//       stop the patch loop and just use this value.
	//       BUT this will fail if one is in a diamond?

	this->apply_value(value, change.get_operation());
}

bool Value::operator ==(const Value &other) const {
	if (typeid(*this) != typeid(other)) {
		return false;
	}
	return this->equals(other);
}

bool Value::operator !=(const Value &other) const {
	return not (*this == other);
}

} // namespace nyan
