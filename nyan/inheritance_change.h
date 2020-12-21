// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include "config.h"
#include "ops.h"


namespace nyan {

/**
 * Runtime inheritance change.
 */
class InheritanceChange {
public:
    InheritanceChange(inher_change_t type, fqon_t &&target);

    /**
     * Get the type of inheritance change, i.e.whether the new parent is appended to
     * the front or the back of the linearization.
     *
     * @return The inheritance change type.
     */
    inher_change_t get_type() const;

    /**
     * Get the object to which the inheritance change is applied.
     *
     * @return Identifier of the target object.
     */
    const fqon_t &get_target() const;

protected:

    /**
     * Inheritance change type.
     */
    inher_change_t type;

    /**
     * Identifier of the target object.
     */
    fqon_t target;
};


} // namespace nyan
