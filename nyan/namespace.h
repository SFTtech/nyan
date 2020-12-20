// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <string>
#include <vector>

#include "config.h"


namespace nyan {

class IDToken;


class Namespace {
    friend struct std::hash<Namespace>;

public:
    explicit Namespace(const fqon_t &token);
    explicit Namespace(const IDToken &token);
    Namespace(const Namespace &other, const std::string &addend);

    virtual ~Namespace() = default;

    void pop_last();
    bool empty() const;

    /**
     * Append the given object/member reference to the namespace identifier to
     * get its identifier.
     *
     * @param name IDToken with an object/member reference.
     * @param skip Number of components at the start of @p name to be skipped.
     *
     * @return Identifier of the object/member.
     */
    fqon_t combine(const IDToken &name, size_t skip=0) const;

    /**
     * Get a (relative) path to a filename for the namespace.
     *
     * @return String representation of the path. Uses '/' as path
     *     component separator.
     */
    std::string to_filename() const;

    /**
     * Create a namespace from a given filename. Performs a sanity
     * check on the filename.
     *
     * @param filename Name of a file, including the extension.
     *
     * @return Namespace for the filename.
     */
    static Namespace from_filename(const std::string &filename);

    /**
     * Get the identifier of the namespace.
     *
     * @return Identifier of the namespace.
     */
    fqon_t to_fqon() const;

    /**
     * Get a string representation of the namespace.
     *
     * @return String representation of the namespace.
     */
    std::string str() const;

    /**
     * Checks if this namespace is equal to a given namespace.
     *
     * @return true if the namespaces are equal, else false.
     */
    bool operator ==(const Namespace &other) const;

protected:
    std::vector<std::string> components;
};

} // namespace nyan


namespace std {
template <>
struct hash<nyan::Namespace> {
    size_t operator ()(const nyan::Namespace &ns) const;
};
} // namespace std
