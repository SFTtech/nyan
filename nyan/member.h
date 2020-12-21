// Copyright 2016-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once


#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>

#include "ops.h"
#include "type.h"
#include "value/value.h"

namespace nyan {


/**
 * Stores a member of a Object.
 * Also responsible for validating applied operators.
 */
class Member {
public:
    /**
     * Member with value.
     */
    Member(override_depth_t depth,
           nyan_op operation,
           Type declared_type,
           ValueHolder &&value);

    Member(const Member &other);
    Member(Member &&other) noexcept;
    Member &operator =(const Member &other);
    Member &operator =(Member &&other) noexcept;

    ~Member() = default;

    /**
     * Get the operation performed by this member.
     *
     * @return Operation of the member.
     */
    nyan_op get_operation() const;

    /**
     * Get the value stored in this member.
     *
     * @return Value of the member.
     */
    const Value &get_value() const;

    /**
     * Apply another member, using its operation, to this member.
     *
     * @param change Member applied to this member.
     */
    void apply(const Member &change);

    /**
     * Get the string representation of this member's initialization part,
     * i.e. operation and value.
     *
     * @return String containing the member initialization in nyan format.
     */
    std::string str() const;

protected:

    /**
     * Number of @ chars before the operation,
     * those define the override depth when applying the patch.
     */
    override_depth_t override_depth = 0;

    /**
     * Operation specified for this member.
     */
    nyan_op operation = nyan_op::INVALID;

    /**
     * Type from the member declaration.
     */
    Type declared_type;

    /**
     * Value stored in this member.
     */
    ValueHolder value;
};


} // namespace nyan
