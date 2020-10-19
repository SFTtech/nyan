// Copyright 2016-2018 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value.h"

#include "boolean.h"
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


static ValueHolder value_from_value_token(const Type &target_type,
                                          const IDToken &value_token,
                                          const std::function<fqon_t(const Type &, const IDToken &)> &get_obj_value) {

	switch (target_type.get_primitive_type()) {
	case primitive_t::BOOLEAN:
		return {std::make_shared<Boolean>(value_token)};

	case primitive_t::TEXT:
		return {std::make_shared<Text>(value_token)};

	case primitive_t::INT: {
		if (value_token.get_type() == token_type::INF) {
			return {std::make_shared<Int>(value_token)};
		}
		else if (value_token.get_type() == token_type::INT) {
			return {std::make_shared<Int>(value_token)};
		}
		else if (value_token.get_type() == token_type::FLOAT) {
			return {std::make_shared<Float>(value_token)};
		}
		throw LangError{
			value_token,
			"invalid token for number, expected float, int or inf"
		};
	}
	case primitive_t::FLOAT: {
		if (value_token.get_type() == token_type::INF) {
			return {std::make_shared<Float>(value_token)};
		}
		else if (value_token.get_type() == token_type::INT) {
			return {std::make_shared<Int>(value_token)};
		}
		else if (value_token.get_type() == token_type::FLOAT) {
			return {std::make_shared<Float>(value_token)};
		}
		throw LangError{
			value_token,
			"invalid token for number, expected float, int or inf"
		};
	}

	case primitive_t::FILENAME: {
		// TODO: make relative to current namespace
		return {std::make_shared<Filename>(value_token)};
	}
	case primitive_t::OBJECT: {

		if (unlikely(value_token.get_type() != token_type::ID)) {
			throw LangError{
				value_token,
				"invalid value for object, expecting object id"
			};
		}

		fqon_t obj_id = get_obj_value(target_type, value_token);

		return {std::make_shared<ObjectValue>(std::move(obj_id))};
	}
	default:
		throw InternalError{"non-implemented value type"};
	}

	return {nullptr};
}


ValueHolder Value::from_ast(const Type &target_type,
                            const ASTMemberValue &astmembervalue,
                            const std::function<fqon_t(const Type &, const IDToken &)> &get_obj_value) {

	// TODO: someday values may be nested more than one level.
	//       then this function must be boosted a bit.

	if (not target_type.is_container()) {
		// don't allow more than one value for a single-value type
		if (astmembervalue.get_values().size() > 1) {
			throw TypeError{
				astmembervalue.get_values()[1],
				"storing multiple values in non-container member"
			};
		}

		return value_from_value_token(target_type,
		                              astmembervalue.get_values()[0],
		                              get_obj_value);
	}

	// process multi-value values (orderedsets etc)
	std::vector<ValueHolder> values;
	values.reserve(astmembervalue.get_values().size());

	// convert all tokens to values
	const Type *element_type = target_type.get_element_type();
	if (unlikely(element_type == nullptr)) {
		throw InternalError{"container element type is nonexisting"};
	}

	for (auto &value_token : astmembervalue.get_values()) {
		values.push_back(
			value_from_value_token(*element_type, value_token,
			                       get_obj_value)
		);
	}

	// switch by container type determined in the ast,
	// which can be different than the target_type.
	switch (astmembervalue.get_container_type()) {
	case container_t::SET:
		// create a set from the value list
		return {std::make_shared<Set>(std::move(values))};

	case container_t::ORDEREDSET:
		return {std::make_shared<OrderedSet>(std::move(values))};

	default:
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
