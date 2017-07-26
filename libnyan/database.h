// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "meta_info.h"
#include "namespace_finder.h"
#include "state.h"


namespace nyan {

class ASTObject;
class File;
class Member;
class Namespace;
class View;



/**
 * Be in a namespace, look up an alias, and get the original namespace.
 * namespace => {alias => original namespace}
 */
using namespace_lookup_t = std::unordered_map<Namespace, NamespaceFinder>;


/**
 * Nyan database.
 */
class Database : std::enable_shared_from_this<Database> {
public:

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
	 * Return the database state.
	 */
	const State &get_state() const {
		return this->state;
	};

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
		const NamespaceFinder &current_file,
		const Namespace &ns,
		const Namespace &objname,
		const ASTObject &astobj
	);

	void create_obj_state(
		const NamespaceFinder &current_file,
		const Namespace &ns,
		const Namespace &objname,
		const ASTObject &astobj
	);

	void linearize_new(const std::vector<fqon_t> &new_objs);

	void find_member(
		bool skip_first,
		const memberid_t &member_id,
		const ObjectState &obj_state,
		const ObjectInfo &obj_info,
		const std::function<bool(const fqon_t &, const MemberInfo &, const Member *)> &member_found
	);

	void resolve_types(const std::vector<fqon_t> &new_objs);

	void check_hierarchy(const std::vector<fqon_t> &new_objs);

	/**
	 * Database start state.
	 * Used as base for the changes, those are tracked in a View.
	 */
	State state;

	/**
	 * Tracks type information and locations of the database content etc.
	 */
	MetaInfo meta_info;

	// namespace tracking?
};

} // namespace nyan
