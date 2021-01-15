// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "value.h"

#include "boolean.h"
#include "dict.h"
#include "file.h"
#include "none.h"
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
static ValueHolder value_from_id_token(const Type &target_type,
                                       const IDToken &id_token,
                                       const std::function<fqon_t(const IDToken &)> &get_fqon) {

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

        fqon_t obj_id = get_fqon(id_token);

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
static std::vector<ValueHolder> value_from_value_token(const std::vector<Type> &target_types,
                                                       const ValueToken &value_token,
                                                       const std::function<fqon_t(const IDToken &)> &get_fqon) {
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

    if (value_token.is_none()) {
        values.emplace_back(std::make_shared<None>(NYAN_NONE));
        return values;
    }

    for (unsigned int i = 0; i < value_token.get_value().size(); i++) {
        values.push_back(value_from_id_token(
                target_types.at(i),
                value_token.get_value().at(i),
                get_fqon
            )
        );
    }

    return values;
}


/**
 * Handle modifiers of the member type, i.e. perform necessary modifications
 * and checks on the created value.
 *
 * @param modifiers List of modifier ordered by inner mod --> outer mod.
 * @param value_holder Value that is modified/checked.
 * @param objs_in_values Pointer to the list of object identifiers in
 *     values at load time.
 */
static void handle_modifiers(const std::vector<Type> &modifiers,
                             const ValueHolder &value_holder,
                             std::vector<std::pair<fqon_t, Location>> *objs_in_values) {
    bool contains_optional = false;
    for (auto &mod: modifiers) {
        auto modifier_type = mod.get_composite_type();

        if (modifier_type == composite_t::OPTIONAL) {
            contains_optional = true;
        }
    }

    if (typeid(*value_holder.get_value()) == typeid(None&) and not contains_optional) {
        throw InternalError{"NoneValue is content, but no optional modifier was found"};
    }

    for (auto &mod: modifiers) {
        auto modifier_type = mod.get_composite_type();

        switch (modifier_type) {
        case composite_t::OPTIONAL: {
            contains_optional = true;
            break;
        }

        case composite_t::CHILDREN: {
            if (unlikely(typeid(*value_holder.get_value()) != typeid(ObjectValue&))) {
                if (not contains_optional) {
                    throw InternalError{"children type requires ObjectValue as content"};
                }
            }

            // Check if object fqon is a child (i.e. not the same fqon as  the member type)
            ObjectValue obj = dynamic_cast<ObjectValue&>(*value_holder.get_value());
            fqon_t member_type_fqon = mod.get_element_type()->at(0).get_fqon();

            if (obj.get() == member_type_fqon) {
                throw InternalError{"children type does not allow an ObjectValue with same fqon as the member type"};
            }
            break;
        }

        case composite_t::ABSTRACT: {
            if (unlikely(typeid(*value_holder.get_value()) != typeid(ObjectValue&))) {
                if (not contains_optional) {
                    throw InternalError{"abstract type requires ObjectValue as content"};
                }
            }

            // Remove last element here, so the object is not checked
            // for non-abstractness later
            objs_in_values->pop_back();
            break;
        }

        default:
            break;
        }
    }
}


ValueHolder Value::from_ast(const Type &target_type,
                            const ASTMemberValue &astmembervalue,
                            std::vector<std::pair<fqon_t, Location>> *objs_in_values,
                            const std::function<fqon_t(const IDToken &)> &get_fqon,
                            const std::function<std::vector<fqon_t>(const fqon_t &)> &get_obj_lin) {
    // Save the modifiers in reverse order
    // we will check them after we receive a value
    Type current_type = target_type;
    std::vector<Type> modifiers;
    while (current_type.is_modifier()) {
        modifiers.insert(modifiers.begin(), current_type);
        current_type = current_type.get_element_type()->at(0);
    }

    if (not current_type.is_container()) {
        // don't allow more than one value for a single-value type
        if (astmembervalue.get_values().size() > 1) {
            throw TypeError{
                astmembervalue.get_values()[1].get_start_location(),
                "storing multiple values in non-container"
            };
        }

        std::vector<Type> target_types{current_type};
        ValueHolder value = value_from_value_token(target_types,
                                                        astmembervalue.get_values()[0],
                                                        get_fqon)[0];

        if (target_type.is_object()) {
            // Check if member type is in object linearization of the value
            ObjectValue obj = dynamic_cast<ObjectValue&>(*value.get_value());
            std::vector<fqon_t> obj_lin = get_obj_lin(obj.get());

            if (unlikely(not util::contains(obj_lin, target_type.get_fqon()))) {
                throw InternalError{"object is not in linearization of member type"};
            }
        }

        // Checks if value adheres to applied modifiers
        handle_modifiers(modifiers, value, objs_in_values);

        return value;
    }

    composite_t composite_type = current_type.get_composite_type();

    switch (composite_type)
    {
    case composite_t::ORDEREDSET:
    case composite_t::SET: {
        std::vector<ValueHolder> values;

        // process multi-value values (orderedsets etc)
        values.reserve(astmembervalue.get_values().size());

        // convert all tokens to values
        const std::vector<Type> *element_type = current_type.get_element_type();
        if (unlikely(element_type == nullptr)) {
            throw InternalError{"container element type is nonexisting"};
        }

        Type cur_elem_type = element_type->at(0);
        std::vector<Type> elem_modifiers;
        while (cur_elem_type.is_modifier()) {
            elem_modifiers.insert(elem_modifiers.begin(), cur_elem_type);
            cur_elem_type = cur_elem_type.get_element_type()->at(0);
        }

        std::vector<Type> target_types{cur_elem_type};
        for (auto &value_token : astmembervalue.get_values()) {
            ValueHolder value = value_from_value_token(
                target_types,
                value_token,
                get_fqon
            )[0];

            handle_modifiers(elem_modifiers, value, objs_in_values);

            values.push_back(value);

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

    case composite_t::DICT: {
        std::unordered_map<ValueHolder, ValueHolder> items;

        items.reserve(astmembervalue.get_values().size());

        // convert all tokens to values
        const std::vector<Type> *element_type = target_type.get_element_type();
        if (unlikely(element_type == nullptr)) {
            throw InternalError{"container element type is nonexisting"};
        }

        Type cur_elem_type_key = element_type->at(0);
        std::vector<Type> elem_modifiers_key;
        while (cur_elem_type_key.is_modifier()) {
            elem_modifiers_key.insert(elem_modifiers_key.begin(), cur_elem_type_key);
            cur_elem_type_key = cur_elem_type_key.get_element_type()->at(0);
        }

        Type cur_elem_type_value = element_type->at(1);
        std::vector<Type> elem_modifiers_value;
        while (cur_elem_type_value.is_modifier()) {
            elem_modifiers_value.insert(elem_modifiers_value.begin(), cur_elem_type_value);
            cur_elem_type_value = cur_elem_type_value.get_element_type()->at(0);
        }

        std::vector<Type> target_types{cur_elem_type_key, cur_elem_type_value};

        for (auto &value_token : astmembervalue.get_values()) {
            std::vector<ValueHolder> keyval = value_from_value_token(
                target_types,
                value_token,
                get_fqon
            );

            handle_modifiers(elem_modifiers_key, keyval[0], objs_in_values);
            handle_modifiers(elem_modifiers_value, keyval[1], objs_in_values);

            items.insert(std::make_pair(keyval[0], keyval[1]));
        }

        return {std::make_shared<Dict>(std::move(items))};
    }

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
