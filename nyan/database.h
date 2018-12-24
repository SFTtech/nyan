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
	 */
	void load(const std::string &filename,
	          const filefetcher_t &filefetcher);

	/**
	 * Return a new view to the database, it allows changes.
	 */
	std::shared_ptr<View> new_view();

	/**
	 * Return the initial database state.
	 */
	const std::shared_ptr<State> &get_state() const {
		return this->state;
	};

	/**
	 * Return the type info storage.
	 */
	const MetaInfo &get_info() const {
		return this->meta_info;
	}

protected:

	void create_obj_info(
		size_t *counter,
		const NamespaceFinder &current_file,
		const Namespace &ns,
		const Namespace &objname,
		const ASTObject &astobj
	);

	void create_obj_content(
		std::vector<fqon_t> *new_objs,
		std::unordered_map<fqon_t, std::unordered_set<fqon_t>> *child_assignments,
		const NamespaceFinder &current_file,
		const Namespace &ns,
		const Namespace &objname,
		const ASTObject &astobj
	);

	void create_obj_state(
		std::vector<std::pair<fqon_t, Location>> *objs_in_values,
		const NamespaceFinder &current_file,
		const Namespace &ns,
		const Namespace &objname,
		const ASTObject &astobj
	);

	void linearize_new(const std::vector<fqon_t> &new_objs);

	void find_member(
		bool skip_first,
		const memberid_t &member_id,
		const std::vector<fqon_t> &search_objs,
		const ObjectInfo &obj_info,
		const std::function<bool(const fqon_t &, const MemberInfo &, const Member *)> &member_found
	);

	void resolve_types(const std::vector<fqon_t> &new_objs);

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
