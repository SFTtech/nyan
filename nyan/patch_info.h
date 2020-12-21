// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <vector>

#include "config.h"

namespace nyan {

/**
 * Information about a patch.
 */
class PatchInfo {
public:
    explicit PatchInfo(fqon_t &&target);
    ~PatchInfo() = default;

    /**
     * Get the identifier of the patch target.
     *
     * @return Identifier of the patch target.
     */
    const fqon_t &get_target() const;

    /**
     * Get the string representation of the metadata information.
     *
     * @return String representation of the metadata information.
     */
    std::string str() const;

protected:
    /**
     * Identifier of the patch target.
     */
    fqon_t target;
};


} // namespace nyan
