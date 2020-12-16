// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "meta_info.h"
#include "namespace_finder.h"


namespace nyan {

class ASTObject;
class File;
class Member;
class Namespace;
class ObjectState;
class State;
class View;



/**
 * Be in a namespace, look up an alias, and get the original namespace.
 * namespace => {alias => original namespace}
 */
using namespace_lookup_t = std::unordered_map<Namespace, NamespaceFinder>;


/**
 * The nyan database. Use this class to interface with nyan.
 * Use the static Database::create() method to obtain a shared_ptr,
 * it is required for db views so they can store pointers.
 */
class Database : public std::enable_shared_from_this<Database> {
public:
    /**
     * Create a new nyan database.
     */
    static std::shared_ptr<Database> create();

    /**
     * Construct an empty nyan database.
     * In order for views to work, the database has to be a std::shared_ptr.
     */
    Database();

    ~Database();

    /**
     * Function that provides a file for a given filename.
     * Used to query and open imported files.
     */
    using filefetcher_t = std::function<std::shared_ptr<File>(const std::string &filename)>;

    /**
     * Load a nyan file.
     * This loads imported files as well.
     *
     * @param filename Filename of the to-be-loaded file.
     * @param filefetcher Function to extract the data from the file.
     */
    void load(const std::string &filename,
              const filefetcher_t &filefetcher);

    /**
     * Return a new view to the database, it allows changes.
     *
     * @return Shared pointer to View on the database.
     */
    std::shared_ptr<View> new_view();

    /**
     * Return the initial database state.
     *
     * @return Shared pointer to initial State of the database.
     */
    const std::shared_ptr<State> &get_state() const {
        return this->state;
    };

    /**
     * Return the type info storage.
     *
     * @return The MetaInfo object for the database content.
     */
    const MetaInfo &get_info() const {
        return this->meta_info;
    }

protected:

    /**
     * Create the metadata information ObjectInfo for an object.
     *
     * @param counter Increments after ObjectInfo has been created. Used to count the created objects.
     * @param current_file NamespaceFinder to check object naming conflicts.
     * @param ns Namespace the object is in?
     * @param objname Namespace created by the object.
     * @param astobj Parsed object data from the AST.
     */
    void create_obj_info(
        size_t *counter,
        const NamespaceFinder &current_file,
        const Namespace &ns,
        const Namespace &objname,
        const ASTObject &astobj
    );

    /**
     * Add an object's content to the metadata information ObjectInfo of
     * an object. This will also create metadata information for the
     * object's members.
     *
     * @param new_objs The object's identifier is appended here. Used to track which objects have been processed.
     * @param child_assignments If the object is a child, it is mapped to its parents' fqons. Used to track children of objects.
     * @param current_file NamespaceFinder to check object naming conflicts.
     * @param ns Namespace the object is in.
     * @param objname Namespace created by the object.
     * @param astobj Parsed object data from the AST.
     */
    void create_obj_content(
        std::vector<fqon_t> *new_objs,
        std::unordered_map<fqon_t, std::unordered_set<fqon_t>> *child_assignments,
        const NamespaceFinder &current_file,
        const Namespace &ns,
        const Namespace &objname,
        const ASTObject &astobj
    );

    /**
     * Create the initial ObjectState of an object.
     *
     * @param objs_in_values Object identifiers in the object's member values that must be non-abstract.
     * @param current_file NamespaceFinder to check object naming conflicts.
     * @param ns Namespace the object is in.
     * @param objname Namespace created by the object.
     * @param astobj Parsed object data from the AST.
     */
    void create_obj_state(
        std::vector<std::pair<fqon_t, Location>> *objs_in_values,
        const NamespaceFinder &current_file,
        const Namespace &ns,
        const Namespace &objname,
        const ASTObject &astobj
    );

    /**
     * Linearizes the parents of all given objects.
     *
     * @param new_objs Identifiers of the objects that should be linearized.
     */
    void linearize_new(const std::vector<fqon_t> &new_objs);

    /**
     * Find a member in a given list of objects and perform an operation on it.
     *
     * @param skip_first If true, the first object in the list is skipped.
     * @param member_id Identifier of the member.
     * @param search_objs List of objects that is searched.
     * @param obj_info ObjectInfo of the initial object spawning the function call.
     * @param member_found Function performed on the member after it has been found.
     */
    void find_member(
        bool skip_first,
        const memberid_t &member_id,
        const std::vector<fqon_t> &search_objs,
        const ObjectInfo &obj_info,
        const std::function<bool(const fqon_t &, const MemberInfo &, const Member *)> &member_found
    );

    /**
     * Resolves types that are inherited by objects and members. This will
     * find the origin types and store them in the child objects' and child
     * members' metadata.
     *
     * @param new_objs Identifiers of the objects which should be resolved.
     */
    void resolve_types(const std::vector<fqon_t> &new_objs);

    /**
     * Sanity check after creating the database, e.g.
     *     - checks if patches or objects are malformed.
     *     - checks if relative operators of members can be performed.
     *     - checks if objects in values are non-abstract.
     *     - checks if all members who have metadata are in the database state.
     *
     * @param new_objs Identifiers of the objects which should be checked.
     * @param objs_in_values Object identifiers in the object's member values that must be non-abstract.
     */
    void check_hierarchy(const std::vector<fqon_t> &new_objs,
                         const std::vector<std::pair<fqon_t, Location>> &objs_in_values);

    /**
     * Database start state.
     * Used as base for the changes, those are tracked in a View.
     */
    std::shared_ptr<State> state;

    /**
     * Tracks type information and locations of the database content etc.
     */
    MetaInfo meta_info;
};

} // namespace nyan
