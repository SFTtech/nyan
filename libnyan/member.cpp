// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "member.h"

#include <iostream>

#include "container.h"
#include "object.h"
#include "util.h"


namespace nyan {


Member::Member(const Location &location,
                       TypeContainer &&type)
	:
	type{std::move(type)},
	operation{nyan_op::INVALID},
	cached_value{nullptr},
	value{},
	location{location} {}


Member::Member(const Location &location,
                       TypeContainer &&type, nyan_op operation,
                       ValueContainer &&value)
	:
	type{std::move(type)},
	operation{operation},
	cached_value{nullptr},
	value{std::move(value)},
	location{location} {

	// TODO: check if type, operation and value can work together
	//       the operator check is missing currently.


	// these operators are allowed to assign the value to the type
	const std::unordered_set<nyan_op> ops = this->value->allowed_operations(this->type->get_basic_type());

	// test if the given operation is compatbile with type and value
	if (ops.find(this->operation) == std::end(ops)) {
		// TODO: better error message, location and information
		//       about the conflicting types and operator
		throw TypeError{location, "incompatible types for operand"};
	}


	static auto type_check = [](const Location &location,
	                            const Type *type,
	                            const Value *value) {

		// special handling for some primitive types
		switch (type->get_primitive_type()) {
		case nyan_primitive_type::OBJECT: {
			// we checked the basic type above,
			// so we can now cast the value
			const Object *obj = dynamic_cast<const Object *>(value);

			if (not type->is_parent_of(obj)) {
				// TODO: better error message and location
				throw TypeError{location, "incompatible object assigned"};
			}

			break;
		}
		default:
		// no special handling for the others.
		break;
		}
	};

	type_check(this->location,
	           this->type.get(),
	           this->value.get());


	// if it is a container, check all container content as well.
	if (this->type->is_container()) {
		// check if all the container values
		// are compatible with the container type

		const Container *container = dynamic_cast<Container *>(this->value.get());

		if (container == nullptr) {
			throw InternalError{
				"type said it was a container but could not be casted!"
			};
		}

		const Type *element_type = this->type->get_element_type();

		for (auto &entry : *container) {

			const nyan_basic_type &value_type = entry.get_type();

			// test if the basic type of the value is compatbile
			if (not element_type->is_basic_compatible(value_type)) {
				// TODO: better error message
				throw TypeError{
					this->location,
					"element with wrong type in container"
				};
			}

			type_check(this->location,
			           element_type,
			           &entry);
		}
	}
}


Member::Member(Member &&other) noexcept
	:
	type{std::move(other.type)},
	operation{other.operation},
	cached_value{std::move(other.cached_value)},
	value{std::move(other.value)},
	location{std::move(other.location)} {}


const Member &Member::operator =(Member &&other) noexcept {
	this->type = std::move(other.type);
	this->operation = other.operation;
	this->cached_value = std::move(other.cached_value);
	this->value = std::move(other.value);
	this->location = std::move(other.location);
	return *this;
}


std::string Member::str() const {
	std::ostringstream builder;

	if (this->type.is_owning()) {
		builder << " : " << this->type->str();
	}

	if (this->operation != nyan_op::INVALID) {
		builder << " " << op_to_string(this->operation);
	}

	if (this->value.has_data()) {
		builder << " " << this->value->repr();
	}

	return builder.str();
}


Type *Member::get_type() const {
	return this->type.get();
}


Value *Member::get_value_ptr() const {
	return this->value.get();
}


nyan_op Member::get_operation() const {
	return this->operation;
}


void Member::cache_save(std::unique_ptr<Value> &&value) {
	this->cached_value = std::move(value);
}


Value *Member::cache_get() const {
	return this->cached_value.get();
}


void Member::cache_reset() {
	this->cached_value.reset(nullptr);
}


const Location &Member::get_location() const {
	return this->location;
}

} // namespace nyan
