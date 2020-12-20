// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>

#include "location.h"


namespace nyan {

class Type;


/**
 * Stores information for a member of an Object.
 */
class MemberInfo {
public:
    explicit MemberInfo(const Location &location);
    ~MemberInfo() = default;

    /**
     * Set the type of this member. Moves the shared pointer storing the
     * type.
     *
     * @param type Shared pointer to the Type.
     * @param initial Set to true if the member defined the type, false if the member is a
     * 	    patch member or inherited.
     *
     * @return Type of the member.
     */
    Type &set_type(std::shared_ptr<Type> &&type, bool initial);

    /**
     * Set the type of this member. Copies the shared pointer storing the
     * type.
     *
     * @param type Shared pointer to the Type.
     * @param initial Set to true if the member defined the type, false if the
     *      member is a patch member or inherited.
     *
     * @return Type of the member.
     */
    Type &set_type(const std::shared_ptr<Type> &type, bool initial);

    /**
     * Returns the type of this member.
     *
     * @return Type of the member.
     */
    const std::shared_ptr<Type> &get_type() const;

    /**
     * Get the position of this member in a file.
     *
     * @return Location of the member.
     */
    const Location &get_location() const;

    /**
     * Checks if this member contains the initial type definition, i.e. it
     * is not a patch member or inherited.
     *
     * @return true if the member is the initial definition, else false.
     */
    bool is_initial_def() const;

    /**
     * Get the string representation of this member's declaration.
     *
     * @return String containing the member declaration in nyan format.
     */
    std::string str() const;

protected:
    /**
     * Location where the member was defined.
     */
    Location location;

    /**
     * Determines whether this member definition is the initial one.
     */
    bool initial_def;

    /**
     * Type of the member.
     */
    std::shared_ptr<Type> type;
};


} // namespace nyan
