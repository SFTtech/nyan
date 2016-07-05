// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_parser.h"

#include "nyan_ast.h"
#include "nyan_file.h"
#include "nyan_flex_lexer.h"
#include "nyan_lexer.h"
#include "nyan_member.h"
#include "nyan_store.h"
#include "nyan_token.h"
#include "nyan_type.h"
#include "nyan_type_container.h"
#include "nyan_util.h"
#include "nyan_value.h"
#include "nyan_value_container.h"
#include "nyan_value_number.h"
#include "nyan_value_orderedset.h"
#include "nyan_value_set.h"
#include "nyan_value_text.h"


namespace nyan {

NyanParser::NyanParser(NyanStore *store)
	:
	store{store} {}


std::vector<NyanObject *> NyanParser::parse(const NyanFile &file) {
	// If you are some parser junkie and I trigger your rage mode now,
	// feel free to rewrite the parser or use a tool like bison.

	// tokenize input
	std::vector<NyanToken> tokens = this->tokenize(file);

	// create ast from tokens
	NyanAST root = this->create_ast(tokens);

	// create objects from tokens
	std::vector<NyanObject *> ret = this->create_objects(root);

	return ret;
}


std::vector<NyanToken> NyanParser::tokenize(const NyanFile &file) const {
	NyanLexer lexer{file};

	std::vector<NyanToken> ret;

	while (true) {
		NyanToken token = lexer.get_next_token();
		bool end = (token.type == token_type::ENDFILE);

		ret.push_back(std::move(token));

		if (end) {
			break;
		}
	}

	return ret;
}


NyanAST NyanParser::create_ast(const std::vector<NyanToken> &tokens) const {
	util::Iterator<NyanToken> token_iter{tokens};
	NyanAST root{token_iter};
	return root;
}


std::vector<NyanObject *> NyanParser::create_objects(const NyanAST &ast) {
	std::vector<NyanObject *> ret;

	// walk over all objects present in the abstract syntax tree
	for (auto &astobj : ast.get_objects()) {

		// create the object, then fill up its contents.
		auto obj = std::make_unique<NyanObject>(
			NyanLocation{astobj.name},
			this->store
		);

		// the object name, valid in its namespace.
		obj->name = astobj.name.get();

		// check if the name is unique
		if (this->store->get(obj->name) != nullptr) {
			// TODO: show other location!
			throw NyanFileError{astobj.name, "object name already used"};
		}

		// first: inheritance parents
		// that way, these can already be queried to get type infos
		this->add_inheritance(obj.get(), astobj);

		// update the inheritance linearization
		obj->generate_linearization();

		// allowed patch targets are stored as __patch__ member
		this->add_patch_targets(obj.get(), astobj);

		// detect if the members have type declarations.
		// if not, take the declaration of some parent to
		// determine the correct type.
		auto member_types = this->member_type_creation(obj.get(), astobj);

		// then, go over them again and assign values,
		// the types must be known by now.
		auto members = this->create_members(obj.get(), astobj, member_types);

		// store inheritance parent modifications
		this->inheritance_mod(obj.get(), astobj);

		// save the object to the target store.
		NyanObject *obj_ptr = this->store->add(std::move(obj));
		ret.push_back(obj_ptr);
	}

	return ret;
}


void NyanParser::add_inheritance(NyanObject *obj,
                                 const NyanASTObject &astobj) const {

	for (auto &parent_name : astobj.inheritance) {
		NyanObject *parent = this->store->get(parent_name.get());
		if (parent == nullptr) {
			throw TypeError{parent_name, "parent not found"};
		}
		// TODO: ordered set creation
		obj->parents.push_back(parent);
	}
}


void NyanParser::add_patch_targets(NyanObject *obj, const NyanASTObject &astobj) const {

	// if patch targets were specified, store them in the __patch__ member
	if (astobj.target.exists()) {

		// fetch the denoted patch target object
		NyanObject *tobj = this->store->get(astobj.target.get());

		if (tobj == nullptr) {
			throw NyanFileError{
				astobj.target,
				"unknown patch target object"
			};
		}

		// create the __patch__ member to contain the target
		NyanMember patch_member{
			NyanLocation{astobj.target},
			NyanTypeContainer{std::make_unique<NyanType>(
				nullptr
				// ^ = nullptr object, so any NyanObject is allowed
			)},
			nyan_op::ASSIGN,
			NyanValueContainer{tobj}
		};

		// create the member entry
		obj->members.emplace("__patch__", std::move(patch_member));
	}
}


std::unordered_map<std::string, NyanTypeContainer> NyanParser::member_type_creation(NyanObject *obj, const NyanASTObject &astobj) const {

	// maps member_name -> type
	std::unordered_map<std::string, NyanTypeContainer> member_types;

	// first, gather all members and detect
	// if they have a type declaration
	// if it's not there, require it to be inferred from some parent
	for (auto &astmember : astobj.members) {

		// test if a member with the same name was already used in the object
		if (member_types.find(astmember.name.get()) != std::end(member_types)) {
			// TODO: show previous location
			throw ASTError{
				"member already used in this object",
				astmember.name
			};
		}

		// type that is inferred from parent objects
		// the new member must be compatible to this type.
		// if nullptr, this member defines the member as a new one.
		NyanType *inferred_type = obj->infer_type(astmember.name.get());

		// TODO: detect if two base classes declare the same member
		//       independently. One member must be defined by exactly
		//       one object, else, it's a name clash which must be
		//       resolved used qualified names.

		// infer type from patch targets and check if they are compatible
		const NyanMember *patch_target_member = obj->get_member_ptr("__patch__");

		NyanType *patch_inferred_type = nullptr;

		// walk up the whole patch tree to find a potential source
		while (patch_target_member != nullptr) {
			// if we have a patch, try using the target object
			// for inferrng member types

			const NyanType *type = patch_target_member->get_type();
			if (not type->is_child_of(NyanType{nullptr})) {
				throw TypeError{
					patch_target_member->get_location(),
					"patch targets must store a NyanObject"
				};
			}

			// get the patch target object
			NyanObject *patch_target = patch_target_member->get_value<NyanObject>();

			patch_inferred_type = patch_target->infer_type(astmember.name.get());

			if (patch_inferred_type != nullptr) {
				// the current patch target declares the queried member,
				// no need to check more of the patches.
				break;
			}
			else {
				// the current patch target didn't define it.
				// but the patch target has another target to patch,
				// so try again with it and see if we can find it there.
				patch_target_member = patch_target->get_member_ptr("__patch__");
			}
		}

		// check if the type could be fetched by the inheritance parents
		if (inferred_type == nullptr) {
			// use the inferred type of the patch target.
			inferred_type = patch_inferred_type;
		}
		else if (patch_inferred_type != nullptr) {
			// a type could be inferred from both the patch target
			// and some parent.
			// check if the patch target member type
			// is more specialized than the by-inheritance type.
			// only then can the patch be applied.
			if (not patch_inferred_type->is_child_of(*inferred_type)) {
				// TODO: show both origins!
				throw TypeError{
					astmember.name.location,
					"incompatible types between a parent object "
					"and a patch target"
				};
			}
		}


		// if there is an explicit type specification
		if (astmember.type.exists) {
			auto explicit_type = std::make_unique<NyanType>(astmember.type, *this->store);

			// check if the explicit type is compatible
			// with the parent one.
			if (inferred_type != nullptr) {
				if (not explicit_type->is_child_of(*inferred_type)) {
					// TODO: show the conflicting location
					throw ASTError{
						"incompatible type",
						astmember.type.name
					};
				}
			}

			// this member has an explicit type specifier, use it.
			member_types.emplace(
				astmember.name.get(),
				NyanTypeContainer{std::move(explicit_type)}
			);
		}
		else {
			// could not find a parent that declares the member type
			if (inferred_type == nullptr) {
				// TODO: and no patch target
				// TODO: which object caused the error
				throw TypeError{
					astmember.name,
					"no parent object declares a type for this member"
				};
			}

			// this member doesn't have a type specifier,
			// but we can take some parent one.
			member_types.emplace(
				astmember.name.get(),
				NyanTypeContainer{inferred_type}
			);
		}
	}

	return member_types;
}


std::vector<std::unique_ptr<NyanMember>> NyanParser::create_members(NyanObject *obj, const NyanASTObject &astobj, std::unordered_map<std::string, NyanTypeContainer> &member_types) const {

	std::vector<std::unique_ptr<NyanMember>> members;

	for (auto &astmember : astobj.members) {
		// now: construct a NyanMember

		NyanValueContainer member_value;

		const std::string &member_name = astmember.name.get();

		// type determined before, either by asking the parents
		// or by explicit user notation
		auto it = member_types.find(member_name);
		if (it == std::end(member_types)) {
			throw NyanInternalError{"unknown member requested"};
		}

		NyanTypeContainer &member_type = it->second;

		// only add a value if the member entry has one
		if (astmember.value.exists) {

			// create the member value
			member_value = this->create_member_value(member_type.get(),
			                                         astmember.value);

			obj->members.emplace(
				member_name,
				NyanMember{
					NyanLocation{astmember.name},
					std::move(member_type),
					astmember.operation,
					std::move(member_value)
				}
			);
		}
		else {
			// no value given
			obj->members.emplace(
				member_name,
				NyanMember{
					NyanLocation{astmember.name},
					std::move(member_type)
				}
			);
		}
	}

	return members;
}


NyanValueContainer NyanParser::create_member_value(const NyanType *member_type, const NyanASTMemberValue &astmembervalue) const {

	NyanValueContainer member_value;

	std::vector<NyanValueContainer> values;

	// don't allow more than one value for a single-value type
	if (astmembervalue.container_type == nyan_container_type::SINGLE and
	    astmembervalue.values.size() > 1) {

		throw TypeError{
			astmembervalue.values[1],
			"storing multiple values in non-container member"
		};
	}

	// convert all tokens to values
	for (auto &value_token : astmembervalue.values) {
		NyanValueContainer value = this->value_from_value_token(value_token);
		values.push_back(std::move(value));
	}

	// switch by container type determined in the ast
	switch (astmembervalue.container_type) {
	case nyan_container_type::SINGLE: {

		member_value = std::move(values[0]);
		break;
	}
	case nyan_container_type::SET: {
		// create a set from the value list
		member_value = NyanValueContainer{
			std::make_unique<NyanSet>(values)
		};
		break;
	}
	case nyan_container_type::ORDEREDSET: {
		member_value = NyanValueContainer{
			std::make_unique<NyanOrderedSet>(values)
		};
		break;
	}

	default:
		throw NyanInternalError{"unhandled container type"};
	}

	return member_value;
}


NyanValueContainer NyanParser::value_from_value_token(const NyanToken &value_token) const {
	NyanValueContainer member_value;

	nyan_type value_type = type_from_value_token(value_token);

	switch (value_type) {
	case nyan_type::TEXT:
		member_value = NyanValueContainer{
			std::make_unique<NyanText>(value_token)
		};
		break;
	case nyan_type::INT:
		member_value = NyanValueContainer{
			std::make_unique<NyanInt>(value_token)
		};
		break;
	case nyan_type::FLOAT: {
		member_value = NyanValueContainer{
			std::make_unique<NyanFloat>(value_token)
		};
		break;
	}
	case nyan_type::OBJECT: {
		NyanObject *obj = this->store->get(value_token.get());
		if (obj == nullptr) {
			throw TypeError{
				value_token,
				"unknown object name"
			};
		}
		member_value = NyanValueContainer{
			obj
		};
		break;
	}
	default:
		throw NyanInternalError{"non-implemented value type"};
	}

	return member_value;
}


void NyanParser::inheritance_mod(NyanObject *obj, const NyanASTObject &astobj) const {
	for (auto &parent_name : astobj.inheritance_add) {
		// TODO: inheritance modifications as __parents_add__
		throw NyanInternalError{"TODO inheritance mod"};
	}
}

} // namespace nyan
