// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "member.h"

#include <iostream>

#include "compiler.h"
#include "util.h"
#include "value/none.h"


namespace nyan {


Member::Member(override_depth_t depth,
               nyan_op operation,
               Type declared_type,
               ValueHolder &&value) :
	override_depth{depth},
	operation{operation},
	declared_type{declared_type},
	value{std::move(value)} {}


Member::Member(const Member &other) :
	override_depth{other.override_depth},
	operation{other.operation},
	declared_type{other.declared_type},
	value{other.value->copy()} {}


Member::Member(Member &&other) noexcept
	:
	override_depth{std::move(other.override_depth)},
	operation{std::move(other.operation)},
	declared_type{std::move(other.declared_type)},
	value{std::move(other.value)} {}


Member &Member::operator=(const Member &other) {
	*this = Member{other};
	return *this;
}


Member &Member::operator=(Member &&other) noexcept {
	this->override_depth = std::move(other.override_depth);
	this->operation = std::move(other.operation);
	this->value = std::move(other.value);
	return *this;
}


nyan_op Member::get_operation() const {
	return this->operation;
}


const Value &Member::get_value() const {
	if (unlikely(not this->value.exists())) {
		throw InternalError{"fetched nonexisting value of member"};
	}

	return *this->value;
}


bool Member::apply(const Member &change) {
	// if the change requests an operator overwrite, apply it.
	if (change.override_depth > 0) {
		// TODO: need to revisit this...
		throw InternalError{"operator overrides not really implemented"};

		this->override_depth = change.override_depth - 1;
		this->operation = change.get_operation();
		// TODO: None can't be copied!
		this->value = change.get_value().copy();
		return true;
	}

	// TODO: if there's more types that require a value override
	// maybe enhance Value::apply and check which type level accepts given operator with value type
	auto &change_value = change.value.get_ptr();

	if (util::isinstance<None>(change_value.get())) {
		// nyan_op::ASSIGN was validated during type check already
		this->value = change_value;
	}
	else if (util::isinstance<None>(*this->value)) {
		if (change.get_operation() == nyan_op::ASSIGN) {
			this->value = change_value;
		}
		// else swallow all operations
	}
	else {
		// keep operator as-is and modify the value.
		return this->value->apply(change);
	}
	return true;
}


std::string Member::str() const {
	std::ostringstream builder;

	if (this->operation != nyan_op::INVALID) {
		builder << op_to_string(this->operation);
	}

	if (this->value.exists()) {
		builder << " " << this->value->repr();
	}

	return std::move(builder).str();
}

} // namespace nyan
