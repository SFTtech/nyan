// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "config.h"
#include "inheritance_change.h"
#include "location.h"
#include "member_info.h"
#include "ops.h"


namespace nyan {

class PatchInfo;
class State;


/**
 * Information about an object.
 * Used for the type system and error messages.
 */
class ObjectInfo {
public:
    using member_info_t = std::unordered_map<memberid_t, MemberInfo>;

    explicit ObjectInfo(const Location &location);
    ~ObjectInfo() = default;

    /**
     * Get the position of this object in a file.
     *
     * @return Location of the object.
     */
    const Location &get_location() const;

    /**
     * Add metadata information for a member.
     *
     * @param name Identifier of the member.
     * @param obj_info MemberInfo with metadata information.
     *
     * @return The stored metadata information object.
     */
    MemberInfo &add_member(const memberid_t &name,
                           MemberInfo &&member);

    /**
     * Get the all metadata information objects for members
     * stored in this object.
     *
     * @return Map of metadata information objects by member identifier.
     */
    member_info_t &get_members();

    /**
     * Get the all metadata information objects for members
     * stored in this object.
     *
     * @return Map of metadata information objects by member identifier.
     */
    const member_info_t &get_members() const;

    /**
     * Get the the metadata information object for a member.
     *
     * @param name Identifier of the member.
     *
     * @return MemberInfo with metadata information if the member is
     *     in the object, else nullptr.
     */
    const MemberInfo *get_member(const memberid_t &name) const;

    /**
     * Add metadata information for a patch if this object is one.
     *
     * @param info Shared pointer to the metadata information object for the patch.
     * @param initial Set to true if this object is the initial patch definition, false if the
     *      object inherits from a patch.
     *
     * @return The stored metadata information object.
     */
    PatchInfo &add_patch(const std::shared_ptr<PatchInfo> &info, bool initial);

    /**
     * Get the the patch metadata information object for this object.
     *
     * @return Shared pointer to the metadata information object for the patch.
     */
    const std::shared_ptr<PatchInfo> &get_patch() const;

    /**
     * Add an inheritance change to the patch target if this is a patch.
     *
     * @param change Inheritance change.
     */
    void add_inheritance_change(InheritanceChange &&change);

    /**
     * Get the list of inheritance changes to the patch target if this is a patch.
     *
     * @return List of inheritance changes made by this patch.
     */
    const std::vector<InheritanceChange> &get_inheritance_change() const;

    /**
     * Set the initial initialization of the object at load time.
     *
     * @param lin C3 linearization of the object as a list of identifiers.
     */
    void set_linearization(std::vector<fqon_t> &&lin);

    /**
     * Get the initial initialization of the object at load time.
     *
     * @return C3 linearization of the object as a list of identifiers.
     */
    const std::vector<fqon_t> &get_linearization() const;

    /**
     * Set the initial children of the object at load time.
     *
     * @param children List of initial children of the object.
     */
    void set_children(std::unordered_set<fqon_t> &&children);

    /**
     * Get the initial direct children of the object at load time.
     *
     * @return List of initial children of the object.
     */
    const std::unordered_set<fqon_t> &get_children() const;

    /**
     * Check if the object is a patch.
     *
     * @return true if the object is a patch, else false.
     */
    bool is_patch() const;

    /**
     * Check if the object is an initial patch, i.e. it is not
     * a patch by inheritance.
     *
     * @return true if the object is a initial patch, else false.
     */
    bool is_initial_patch() const;

    /**
     * Get the string representation of the metadata information.
     *
     * @return String representation of the metadata information.
     */
    std::string str() const;

protected:
    /**
     * Location where the object was defined.
     */
    Location location;

    /**
     * Determines whether this object was defined as a patch.
     * It is one when it was declared with <target>.
     * Otherwise we just link to the parent that does.
     */
    bool initial_patch;

    /**
     * Patch target and modification information.
     */
    std::shared_ptr<PatchInfo> patch_info;

    /**
     * List of objects to add to the patch target.
     */
    std::vector<InheritanceChange> inheritance_change;

    /**
     * Map of member metadata information by their identifier.
     */
    member_info_t member_info;

    /**
     * Linearizations for the object at load time.
     */
    std::vector<fqon_t> initial_linearization;

    /**
     * Direct children of the object at load time.
     */
    std::unordered_set<fqon_t> initial_children;
};


} // namespace nyan
