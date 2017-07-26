// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "database.h"

#include <memory>
#include <unordered_map>
#include <queue>

#include "c3.h"
#include "error.h"
#include "file.h"
#include "namespace.h"
#include "object_state.h"
#include "parser.h"
#include "patch_info.h"
#include "util.h"
#include "view.h"


namespace nyan {

Database::Database() {}


Database::~Database() {}


/**
 * Called for each object.
 * Contains the scope, current namespace,
 * name of the object, and the astobject.
 */
using ast_objwalk_cb_t = std::function<void(const NamespaceFinder &,
                                            const Namespace &,
                                            const Namespace &,
                                            const ASTObject &)>;


static void ast_obj_walk_recurser(const ast_objwalk_cb_t &callback,
                                  const NamespaceFinder &scope,
                                  const Namespace &ns,
                                  const std::vector<ASTObject> &objs) {

	// go over all objects
	for (auto &astobj : objs) {
		Namespace objname{ns, astobj.get_name().get()};

		// process nested objects first
		ast_obj_walk_recurser(callback, scope, objname, astobj.get_objects());

		// do whatever needed
		callback(scope, ns, objname, astobj);
	}
}


static void ast_obj_walk(const namespace_lookup_t &imports,
                         const ast_objwalk_cb_t &cb) {

	// go over all the imported files
	for (auto &it : imports) {
		const Namespace &ns = it.first;
		const NamespaceFinder &current_file = it.second;
		const AST &ast = current_file.get_ast();

		// each file has many objects, which can be nested.
		ast_obj_walk_recurser(cb, current_file, ns, ast.get_objects());
	}
}


void Database::load(const std::string &filename,
                    const filefetcher_t &filefetcher) {

	Parser parser;

	// tracking of imported namespaces (with aliases)
	namespace_lookup_t imports;

	// namespaces to which were requested to be imported
	// the location is the first request origin.
	std::unordered_map<Namespace, Location> to_import;

	// push the first namespace to import
	to_import.insert(
		{
			Namespace::from_filename(filename),
			Location{"explicit load request"}
		}
	);

	while (to_import.size() > 0) {
		auto cur_ns_it = to_import.begin();
		const Namespace &namespace_to_import = cur_ns_it->first;
		const Location &req_location = cur_ns_it->second;

		std::cout << "loading file " << namespace_to_import.to_filename() << std::endl;

		auto it = imports.find(namespace_to_import);
		if (it != std::end(imports)) {
			// this namespace is already imported!
			continue;
		}

		std::shared_ptr<File> current_file;
		try {
			// get the data and parse the file
			current_file = filefetcher(
				namespace_to_import.to_filename()
			);
		}
		catch (FileReadError &err) {
			// the import request failed,
			// so the nyan file structure or content is wrong.
			throw FileError{req_location, err.str()};
		}

		// create import tracking entry for this file
		// and parse the file contents!
		NamespaceFinder &new_ns = imports.insert({
			namespace_to_import,                         // name of the import
			NamespaceFinder{
				parser.parse(current_file)  // read the ast!
			}
		}).first->second;

		// enqueue all new imports of this file
		// and record import aliases
		for (auto &import : new_ns.get_ast().get_imports()) {
			Namespace request{import.get()};

			// either register the alias
			if (import.has_alias()) {
				new_ns.add_alias(import.get_alias(), request);
			}
			// or the plain import
			else {
				new_ns.add_import(request);
			}

			// check if this import was already requested or is known.
			// todo: also check if that ns is already fully loaded in the db
			auto was_imported = imports.find(request);
			auto import_requested = to_import.find(request);

			if (was_imported == std::end(imports) and
			    import_requested == std::end(to_import)) {

				// add the request to the pending imports
				to_import.insert({std::move(request), import.get()});
			}
		}

		to_import.erase(cur_ns_it);
	}


	for (auto &it : imports) {
		std::cout << it.first.str() << " has known info:" << std::endl
		          << it.second.str() << std::endl;
	}

	using namespace std::placeholders;

	size_t new_obj_count = 0;

	// first run: create empty object info objects
	ast_obj_walk(imports, std::bind(&Database::create_obj_info,
	                                this, &new_obj_count,
	                                _1, _2, _3, _4));

	std::vector<fqon_t> new_objects;
	new_objects.reserve(new_obj_count);

	// now, all new object infos need to be filled with types
	ast_obj_walk(imports, std::bind(&Database::create_obj_content,
	                                this, &new_objects,
	                                _1, _2, _3, _4));

	// linearize the parents of all new objects
	this->linearize_new(new_objects);

	// resolve the types of members to their definition
	this->resolve_types(new_objects);


	// state value creation
	ast_obj_walk(imports, std::bind(&Database::create_obj_state,
	                                this,
	                                _1, _2, _3, _4));

	// verify hierarchy consistency
	this->check_hierarchy(new_objects);

	/*
	 * The patch will fail to be loaded if:
	 * The patch target is not known
	 * Any of changed members is not present in the patch target
	 * Any of the added parents is not known
	 * -> Blind patching is not allowed
	 * The patch will succeed to load if:
	 * The patch target already inherits from a parent to be added
	 * -> Inheritance patching doesn't conflict with other patches
	 */

	std::cout << std::endl << "METAINFO:" << std::endl
	          << this->meta_info.str() << std::endl;

	std::cout << std::endl << "INITIAL STATE:" << std::endl
	          << this->state.str() << std::endl;

	// TODO: check pending objectvalues (probably not needed as they're all loaded)
}


void Database::create_obj_info(size_t *counter,
                               const NamespaceFinder &current_file,
                               const Namespace &,
                               const Namespace &objname,
                               const ASTObject &astobj) {

	const std::string &name = astobj.name.get();

	// object name must not be an alias
	if (current_file.check_conflict(name)) {
		// TODO: show conflict origin (the import)
		throw NameError{
			astobj.name,
			"object name conflicts with import",
			name
		};
	}

	this->meta_info.add_object(
		objname.to_fqon(),
		ObjectInfo{astobj.name}
	);

	*counter += 1;
}


void Database::create_obj_content(std::vector<fqon_t> *new_objs,
                                  const NamespaceFinder &scope,
                                  const Namespace &ns,
                                  const Namespace &objname,
                                  const ASTObject &astobj) {

	fqon_t obj_fqon = objname.to_fqon();
	new_objs->push_back(obj_fqon);

	ObjectInfo *info = this->meta_info.get_object(obj_fqon);
	if (unlikely(info == nullptr)) {
		throw InternalError{"object info could not be retrieved"};
	}

	// save the patch target, has to be alias-expanded
	const IDToken &target = astobj.target;
	if (target.exists()) {
		fqon_t target_id = scope.find(ns, target, this->meta_info);
		info->add_patch(std::make_shared<PatchInfo>(std::move(target_id)), true);
	}

	// a patch may add inheritance parents
	for (auto &new_parent : astobj.inheritance_add) {
		fqon_t new_parent_id = scope.find(ns, new_parent, this->meta_info);
		info->add_inheritance_add(std::move(new_parent_id));
	}

	// parents are stored in the object data state
	std::vector<fqon_t> object_parents;
	for (auto &parent : astobj.parents) {
		fqon_t parent_id = scope.find(ns, parent, this->meta_info);
		object_parents.push_back(std::move(parent_id));
	}

	// fill initial state:
	this->state.add_object(
		obj_fqon,
		std::make_shared<ObjectState>(
			std::move(object_parents)
		)
	);

	// create member types
	for (auto &astmember : astobj.members) {

		// TODO: the member name requires advanced expansions
		//       for conflict resolving

		MemberInfo &member_info = info->add_member(
			astmember.name.str(),
			MemberInfo{astmember.name}
		);

		if (not astmember.type.exists()) {
			continue;
		}

		// if existing, create type information of member.
		member_info.set_type(
			std::make_shared<Type>(
				astmember.type,
				scope,
				ns,
				this->meta_info
			),
			true   // type was defined in the ast -> initial definition
		);
	}
}


void Database::linearize_new(const std::vector<fqon_t> &new_objects) {
	// linearize the parents of all newly created objects
	std::unordered_set<fqon_t> linearized_objects;

	for (auto &obj : new_objects) {

		if (linearized_objects.find(obj) != std::end(linearized_objects)) {
			continue;
		}

		std::unordered_set<fqon_t> seen;

		linearize_recurse(
			obj,
			[this] (const fqon_t &name) -> ObjectState& {
				return *this->state.get(name);
			},
			&seen
		);

#if __cplusplus > 201402L  // c++17
		linearized_objects.merge(std::move(seen));
#else
		linearized_objects.insert(std::begin(seen), std::end(seen));
#endif
	}
}


void Database::find_member(bool skip_first,
                           const memberid_t &member_id,
                           const ObjectState &obj_state,
                           const ObjectInfo &obj_info,
                           const std::function<bool(const fqon_t &, const MemberInfo &, const Member *)> &member_found) {

	bool finished = false;

	// member doesn't have type yet. find it.
	for (auto &parent : obj_state.get_linearization()) {

		// at the very beginning, we have to skip the object
		// we want to find the type for. it's the first in the linearization.
		if (skip_first) {
			skip_first = false;
			continue;
		}

		ObjectInfo *parent_info = this->meta_info.get_object(parent);
		if (unlikely(parent_info == nullptr)) {
			throw InternalError{"object information not retrieved"};
		}
		const MemberInfo *parent_member_info = parent_info->get_member(member_id);

		// parent doesn't have this member
		if (not parent_member_info) {
			continue;
		}

		const ObjectState *par_state = this->state.get(parent).get();
		if (unlikely(par_state == nullptr)) {
			throw InternalError{"object state not retrieved"};
		}
		const Member *member = par_state->get_member(member_id);

		finished = member_found(parent, *parent_member_info, member);

		if (finished) {
			break;
		}
	}

	// recurse into the patch target
	if (not finished and obj_info.is_patch()) {
		const fqon_t &target = obj_info.get_patch()->get_target();
		const ObjectInfo *obj_info = this->meta_info.get_object(target);
		const ObjectState *obj_state = this->state.get(target).get();

		// recurse into the target.
		// check if the patch defines the member as well -> error.
		// otherwise, infer type from patch.
		this->find_member(false, member_id, *obj_state, *obj_info, member_found);
	}
}


void Database::resolve_types(const std::vector<fqon_t> &new_objects) {

	using namespace std::string_literals;

	// TODO: if inheritance parents are added,
	//       should a patch be able to modify the newly accessible members?

	// link patch information to the origin patch
	// and check if there's not multiple patche targets per object hierarchy
	for (auto &obj : new_objects) {
		ObjectInfo *obj_info = this->meta_info.get_object(obj);
		ObjectState *obj_state = this->state.get(obj).get();

		const auto &parents = obj_state->get_linearization();
		auto it = std::begin(parents);
		++it; // skip first, it's the object itself.
		for (auto end = std::end(parents); it != end; ++it) {
			ObjectInfo *parent_info = this->meta_info.get_object(*it);

			if (parent_info->is_initial_patch()) {
				if (unlikely(obj_info->is_initial_patch())) {
					// TODO: show patch target instead of member
					throw ReasonError{
						obj_info->get_location(),
						"child patches can't declare a patch target",
						{{parent_info->get_location(), "parent that declares the patch"}}
					};
				}
				else {
					// this is patch because of inheritance.
					// false => it wasn't initially a patch.
					obj_info->add_patch(parent_info->get_patch(), false);
				}
			}
		}
	}

	// resolve member types:
	// link member types to matching parent if not known yet.
	// this required that patch targets are linked.
	for (auto &obj : new_objects) {
		ObjectInfo *obj_info = this->meta_info.get_object(obj);
		ObjectState *obj_state = this->state.get(obj).get();

		// resolve the type for each member
		for (auto &it : obj_info->get_members()) {
			const memberid_t &member_id = it.first;
			MemberInfo &member_info = it.second;

			// if the member already defines it, we found it already.
			// we still need to check for conflicts though.
			bool type_found = member_info.is_initial_def();

			// start the recursion into the inheritance tree,
			// which includes the recursion into patch targets.
			this->find_member(
				true,  // make sure the object we search the type for isn't checked with itself.
				member_id, *obj_state, *obj_info,
				[&member_info, &type_found, &member_id]
				(const fqon_t &parent,
				 const MemberInfo &source_member_info,
				 const Member *) {

					if (source_member_info.is_initial_def()) {
						const std::shared_ptr<Type> &new_type = source_member_info.get_type();

						if (unlikely(not new_type.get())) {
							throw InternalError{"initial type definition has no type"};
						}

						// check if the member we're looking for isn't already typed.
						if (unlikely(member_info.is_initial_def())) {
							// another parent defines this type,
							// which is disallowed!

							// TODO: show location of infringing type instead of member
							throw ReasonError{
								member_info.get_location(),
								("parent '"s + parent
								 + "' already defines type of '" + member_id + "'"),
								{{source_member_info.get_location(), "parent that declares the member"}}
							};
						}

						type_found = true;
						member_info.set_type(new_type, false);
					}
					// else that member knows the type,
					// but we're looking for the initial definition.

					// we need to traverse all members and never stop early.
					return false;
				}
			);

			if (unlikely(not type_found)) {
				throw TypeError{
					member_info.get_location(),
					"could not infer type of '"s + member_id
					+ "' from parents or patch target"
				};
			}
		}
	}
}


void Database::create_obj_state(const NamespaceFinder &scope,
                                const Namespace &ns,
                                const Namespace &objname,
                                const ASTObject &astobj) {

	if (astobj.members.size() == 0) {
		// no members, nothing to do.
		return;
	}

	ObjectInfo *info = this->meta_info.get_object(objname.to_fqon());
	if (unlikely(info == nullptr)) {
		throw InternalError{"object info could not be retrieved"};
	}

	ObjectState &objstate = *this->state.get(objname.to_fqon());

	std::unordered_map<memberid_t, Member> members;

	// create member values
	for (auto &astmember : astobj.members) {

		// member has no value
		if (not astmember.value.exists()) {
			continue;
		}

		// TODO: the member name may need some resolution for conflicts
		const memberid_t &memberid = astmember.name.str();

		const MemberInfo *member_info = info->get_member(memberid);
		if (unlikely(member_info == nullptr)) {
			throw InternalError{"member info could not be retrieved"};
		}

		const Type *member_type = member_info->get_type().get();
		if (unlikely(member_type == nullptr)) {
			throw InternalError{"member type could not be retrieved"};
		}

		nyan_op operation = astmember.operation;

		if (unlikely(operation == nyan_op::INVALID)) {
			// the ast buildup should have ensured this.
			throw InternalError{"member has value but no operator"};
		}

		// create the member with operation and value
		Member &new_member = members.emplace(
			memberid,
			Member{
				0,          // TODO: get override depth from AST (the @-count)
				operation,
				Value::from_ast(*member_type, astmember.value,
				                scope, ns,
				                this->meta_info,
				                this->state)
			}
		).first->second;

		const Value &new_value = new_member.get_value();

		const std::unordered_set<nyan_op> &allowed_ops = new_value.allowed_operations(*member_type);

		if (unlikely(allowed_ops.find(operation) == std::end(allowed_ops))) {
			// TODO: show location of operation
			// TODO: show allowed ones?
			throw TypeError{astmember.name, "invalid operator for this member type"};
		}
	}

	objstate.set_members(std::move(members));
}


void Database::check_hierarchy(const std::vector<fqon_t> &new_objs) {
	for (auto &obj : new_objs) {

		ObjectInfo *obj_info = this->meta_info.get_object(obj);
		ObjectState *obj_state = this->state.get(obj).get();
		if (unlikely(obj_info == nullptr)) {
			throw InternalError{"object info could not be retrieved"};
		}
		if (unlikely(obj_state == nullptr)) {
			throw InternalError{"initial object state could not be retrieved"};
		}

		// check if an object has inher parent adds, it must be a patch.
		if (obj_info->get_inheritance_add().size() > 0) {
			if (unlikely(not obj_info->is_patch())) {
				throw FileError{
					obj_info->get_location(),
					"Inheritance additions can only be done in patches."
				};
			}
		}


		// check that relative operators can't be performed when the parent has no value.
		for (auto &it : obj_state->get_members()) {
			bool assign_ok = false;
			bool other_op = false;

			this->find_member(
				false, it.first, *obj_state, *obj_info,
				[&assign_ok, &other_op] (const fqon_t &,
				                         const MemberInfo &,
				                         const Member *member) {
					// member has no value
					if (member == nullptr) {
						return false;
					}

					nyan_op op = member->get_operation();
					if (op == nyan_op::ASSIGN) {
						assign_ok = true;
						return true;
					}
					else if (likely(op != nyan_op::INVALID)) {
						other_op = true;
					}
					else {
						// op == INVALID
						throw InternalError{"member has invalid operator"};
					}
					return false;
				}
			);

			if (unlikely(other_op and not assign_ok)) {
				const MemberInfo *member_info = obj_info->get_member(it.first);
				throw FileError{
					member_info->get_location(),
					"this member was never assigned a value."
				};
			}
		}

		// TODO: check the @-propagation is type-compatible for each operator
		//       -> can we even know? yes, as the patch target depth must equal @-count.
	}
}


std::shared_ptr<View> Database::new_view() {
	return std::make_shared<View>(shared_from_this());
}


} // namespace nyan
