// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_member.h"

#include <iostream>

#include "nyan_container.h"
#include "nyan_object.h"
#include "nyan_util.h"


namespace nyan {


NyanMember::NyanMember(const NyanLocation &location,
                       NyanTypeContainer &&type)
	:
	type{std::move(type)},
	operation{nyan_op::INVALID},
	cached_value{nullptr},
	value{},
	location{location} {}


NyanMember::NyanMember(const NyanLocation &location,
                       NyanTypeContainer &&type, nyan_op operation,
                       NyanValueContainer &&value)
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


	static auto type_check = [](const NyanLocation &location,
	                            const NyanType *type,
	                            const NyanValue *value) {

		// special handling for some primitive types
		switch (type->get_primitive_type()) {
		case nyan_primitive_type::OBJECT: {
			// we checked the basic type above,
			// so we can now cast the value
			const NyanObject *obj = dynamic_cast<const NyanObject *>(value);

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

		const NyanContainer *container = dynamic_cast<NyanContainer *>(this->value.get());

		if (container == nullptr) {
			throw NyanInternalError{
				"type said it was a container but could not be casted!"
			};
		}

		const NyanType *element_type = this->type->get_element_type();

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


NyanMember::NyanMember(NyanMember &&other)
	:
	type{std::move(other.type)},
	operation{other.operation},
	cached_value{std::move(other.cached_value)},
	value{std::move(other.value)},
	location{std::move(other.location)} {}


const NyanMember &NyanMember::operator =(NyanMember &&other) {
	this->type = std::move(other.type);
	this->operation = other.operation;
	this->cached_value = std::move(other.cached_value);
	this->value = std::move(other.value);
	this->location = std::move(other.location);
	return *this;
}


std::string NyanMember::str() const {
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


NyanType *NyanMember::get_type() const {
	return this->type.get();
}


NyanValue *NyanMember::get_value_ptr() const {
	return this->value.get();
}


nyan_op NyanMember::get_operation() const {
	return this->operation;
}


void NyanMember::cache_save(std::unique_ptr<NyanValue> &&value) {
	this->cached_value = std::move(value);
}


NyanValue *NyanMember::cache_get() const {
	return this->cached_value.get();
}


void NyanMember::cache_reset() {
	this->cached_value.reset(nullptr);
}


const NyanLocation &NyanMember::get_location() const {
	return this->location;
}

} // namespace nyan
