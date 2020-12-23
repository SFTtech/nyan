// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "member.h"

#include <iostream>

#include "compiler.h"
#include "util.h"
#include "value/none.h"


namespace nyan {


Member::Member(override_depth_t depth,
               nyan_op operation,
               Type declared_type,
               ValueHolder &&value)
    :
    override_depth{depth},
    operation{operation},
    declared_type{declared_type},
    value{std::move(value)} {}


Member::Member(const Member &other)
    :
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


Member &Member::operator =(const Member &other) {
    *this = Member{other};
    return *this;
}


Member &Member::operator =(Member &&other) noexcept {
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


void Member::apply(const Member &change) {
    // if the change requests an operator overwrite, apply it.
    if (change.override_depth > 0) {
        this->override_depth = change.override_depth - 1;
        this->operation = change.get_operation();
        this->value = change.get_value().copy();
    }
    else if (typeid(change.get_value()) == typeid(None&)) {
        this->value = {std::make_shared<None>(NYAN_NONE)};
    }
    // else, keep operator as-is and modify the value.
    else {
        this->value->apply(change);
    }
}


std::string Member::str() const {
    std::ostringstream builder;

    if (this->operation != nyan_op::INVALID) {
        builder << op_to_string(this->operation);
    }

    if (this->value.exists()) {
        builder << " " << this->value->repr();
    }

    return builder.str();
}

} // namespace nyan
