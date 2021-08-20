// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value.h"

#include "boolean.h"
#include "dict.h"
#include "file.h"
#include "none.h"
#include "number.h"
#include "nyan/basic_type.h"
#include "nyan/type.h"
#include "nyan/util.h"
#include "object.h"
#include "orderedset.h"
#include "set.h"
#include "text.h"
#include "../ast.h"
#include "../error.h"
#include "../member.h"
#include "../token.h"


namespace nyan {


/**
 * Create a ValueHolder from an IDToken.
 *
 * @param target_types Target type of the value.
 * @param id_token IDToken from which values are extracted.
 * @param get_fqon Function for retrieving an object identifier from
 *     an IDToken.
 *
 * @return A ValueHolder with the created value.
 */
static ValueHolder value_from_id_token(
	const Type &target_type,
	const IDToken &id_token,
	const std::function<fqon_t(const Type &, const IDToken &)> &get_fqon) {

	auto &token_components = id_token.get_components();
	if (target_type.has_modifier(modifier_t::OPTIONAL)
	    and token_components.size() == 1
	    and token_components[0].get() == "None") {

		// link to global None value
		return {None::value};
	}

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

		fqon_t obj_id = get_fqon(target_type, id_token);

		return {std::make_shared<ObjectValue>(std::move(obj_id))};
	}
	default:
		throw InternalError{"non-implemented primitive value type"};
	}

	return {nullptr};
}


/**
 * Create ValueHolders from a ValueToken.
 *
 * @param target_types List of target types of the values in the token. Must have
 *     the same size as the value token.
 * @param value_token Value token from which values are extracted.
 * @param get_fqon Function for retrieving an object identifier from
 *     an IDToken.
 *
 * @return A list of ValueHolders with the created values.
 */
static std::vector<ValueHolder> value_from_value_token(
	const std::vector<Type> &target_types,
	const ValueToken &value_token,
	const std::function<fqon_t(const Type &, const IDToken &)> &get_fqon) {

	auto &&tok_values = value_token.get_value();

	if (unlikely(target_types.size() != tok_values.size())) {
		throw TypeError(
			value_token.get_start_location(),
			std::string("ValueToken has ")
			+ std::to_string(tok_values.size())
			+ " elements, but only "
			+ std::to_string(target_types.size())
			+ " have been requested"
		);
	}

	std::vector<ValueHolder> values;
	values.reserve(tok_values.size());

	for (unsigned int i = 0; i < tok_values.size(); i++) {
		values.push_back(
			value_from_id_token(
				target_types.at(i),
				value_token.get_value().at(i),
				get_fqon
			)
		);
	}

	return values;
}


ValueHolder Value::from_ast(
	const Type &target_type,
	const ASTMemberValue &astmembervalue,
	const std::function<fqon_t(const Type &, const IDToken &)> &get_fqon,
	const std::function<std::vector<fqon_t>(const fqon_t &)> &get_obj_lin) {

	using namespace std::string_literals;

	ValueHolder value;

	const std::vector<ValueToken> &astvalues = astmembervalue.get_values();

	// single value; not a set, dict, ...
	if (not target_type.is_container()) {
		// don't allow more than one value for a single-value type
		if (astvalues.size() > 1) {
			throw TypeError{
				astvalues[1].get_start_location(),
				"storing multiple values in non-container member"
			};
		}

		value = value_from_value_token(
			{target_type},
			astvalues[0],
			get_fqon
		)[0];
	}
	else {

		// now for containers (dict, set, orderedset, ...)
		composite_t composite_type = target_type.get_composite_type();

		switch (composite_type)
		{
		case composite_t::ORDEREDSET:
		case composite_t::SET: {
			std::vector<ValueHolder> values;

			// process multi-value values (orderedsets etc)
			values.reserve(astvalues.size());

			// convert all tokens to values
			const Type &element_type = target_type.get_element_type()[0];

			for (auto &value_token : astvalues) {
				ValueHolder value = value_from_value_token(
					{element_type},
					value_token,
					get_fqon
				)[0];

				if (auto error = value->compatible_with(element_type, get_obj_lin)) {
					throw TypeError(
						value_token.get_start_location(),
						"set element type "s
						+ element_type.str()
						+ " can't be assigned a value of type "
						+ value->get_type().str()
						+ ": " + error->msg
					);
				}

				values.push_back(value);
			}

			switch (composite_type) {
			case composite_t::SET:
				// create a set from the value list
				value = std::make_shared<Set>(std::move(values));
				break;

			case composite_t::ORDEREDSET:
				value = std::make_shared<OrderedSet>(std::move(values));
				break;

			default:
				throw InternalError{"value creation for unhandled container type"};
			}
		} break;

		case composite_t::DICT: {
			std::unordered_map<ValueHolder, ValueHolder> items;

			items.reserve(astvalues.size());

			// convert all tokens to values
			const std::vector<Type> &element_type = target_type.get_element_type();

			const Type &key_type = element_type[0];
			const Type &value_type = element_type[1];

			for (auto &value_token : astvalues) {
				std::vector<ValueHolder> keyval = value_from_value_token(
					element_type,
					value_token,
					get_fqon
				);

				if (auto error = keyval[0]->compatible_with(key_type, get_obj_lin)) {
					throw TypeError(
						value_token.get_start_location(),
						"dict key type "s
						+ key_type.str()
						+ " can't be assigned a value of type "
						+ keyval[0]->get_type().str()
						+ ": " + error->msg
					);
				}
				if (auto error = keyval[1]->compatible_with(value_type, get_obj_lin)) {
					throw TypeError(
						value_token.get_start_location(),
						"dict value type "s
						+ value_type.str()
						+ " can't be assigned a value of type "
						+ keyval[1]->get_type().str()
						+ ": " + error->msg
					);
				}

				items.insert(std::make_pair(keyval[0], keyval[1]));
			}

			value = std::make_shared<Dict>(std::move(items));
		} break;

		default:
			throw InternalError{"value creation for unhandled container type"};
		}
	}

	if (auto error = value->compatible_with(target_type, get_obj_lin)) {
		throw TypeError{
			astvalues[0].get_start_location(),
			"member type "s
			+ target_type.str()
			+ " can't be assigned a value of type "
			+ value->get_type().str()
			+ (error->msg.size() ? ": "s + error->msg : "")
			// TODO: put more info in the error, e.g. underlining the operator
		};
	}

	return value;
}


std::optional<Value::TypeProblem> Value::compatible_with(
	const Type &type,
	const std::function<std::vector<fqon_t>(const fqon_t &)> &get_obj_lin
) const {
	using namespace std::string_literals;

	if (type.has_modifier(modifier_t::OPTIONAL)) {
		if (util::isinstance<None>(*this)) {
			return std::nullopt;
		}
	}

	if (type.is_fundamental()) {
		switch (type.get_basic_type().primitive_type) {
		case primitive_t::BOOLEAN:
			if (not util::isinstance<Boolean>(*this)) {
				return TypeProblem{};
			}
			break;
		case primitive_t::TEXT:
			if (not util::isinstance<Text>(*this)) {
				return TypeProblem{};
			}
			break;
		case primitive_t::FILENAME:
			if (not util::isinstance<Filename>(*this)) {
				return TypeProblem{};
			}
			break;
		case primitive_t::INT:
		case primitive_t::FLOAT:
			if (not util::isinstance<NumberBase>(*this)) {
				return TypeProblem{};
			}
			break;
		case primitive_t::NONE:
			throw InternalError{"type must not have basic type 'none'"};
		default:
			throw InternalError{"unhandled fundamental type"};
		}
		return std::nullopt;
	}
	else if (type.is_container()) {
		// TODO: maybe more sophisticated checks?
		//       especially when we have containers within containers (not supported yet..)
		//       for now, rely on the operator checks...

		if (type.get_composite_type() == composite_t::DICT) {
			if (util::isinstance<Dict>(*this)) {
				return std::nullopt;
			}
		}

		if (not util::isinstance<Container>(*this)) {
			return TypeProblem{"is not a container"};
		}

		return std::nullopt;
	}
	else if (type.is_object()) {
		// check if the value type extends the member type
		const ObjectValue *obj = dynamic_cast<const ObjectValue *>(this);
		if (unlikely(obj == nullptr)) {
			throw InternalError{"type said value is object, but it could not be casted"};
		}

		std::vector<fqon_t> obj_lin = get_obj_lin(obj->get_name());

		if (unlikely(not util::contains(obj_lin, type.get_fqon()))) {
			return TypeProblem{"value does not related to member type object"};
		}

		if (type.has_modifier(modifier_t::CHILDREN)) {
			if (type.get_fqon() == obj->get_name()) {
				return TypeProblem{"only children of '"s + type.get_fqon() + "' are allowed"};
			}
		}
		return std::nullopt;
	}

	throw InternalError{"Value::compatible_with check executed for unhandled type"};
}


bool Value::apply(const Member &change) {
	// extract the member's value,
	// this is just the data of the member,
	// no parent data is included.
	const Value &value = change.get_value();

	// TODO: cache usage: if the value has a cached value,
	//       stop the patch loop and just use this value.
	//       BUT this will fail if one is in a diamond?

	return this->apply_value(value, change.get_operation());
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
