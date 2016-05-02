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
#include "nyan_util.h"
#include "nyan_value.h"
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

		obj->name = astobj.name.get();

		// first: inheritance parents
		// that way, these can already be queried to get type infos
		add_inheritance(obj.get(), astobj);

		// detect if the members have type declarations.
		// if not, take the declaration of some parent to
		// determine the correct type.
		auto member_types = this->member_type_creation(obj.get(), astobj);

		// then, go over them again and assign values,
		// the types must be known by now.
		auto members = this->create_members(obj.get(), astobj, member_types);

		// allowed patch targets are stored as __patch__ member
		this->add_patch_targets(obj.get(), astobj);

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


std::unordered_map<std::string, NyanTypeContainer> NyanParser::member_type_creation(NyanObject *obj, const NyanASTObject &astobj) const {

	// maps member_name -> type
	std::unordered_map<std::string, NyanTypeContainer> member_types;

	// first, gather all members and detect
	// if they have a type declaration
	// if it's not there, require it to be inferred from some parent
	for (auto &astmember : astobj.members) {
		// type that is inferred from parent objects
		// the new member must be compatible to this type.
		// if nullptr, this member defines the member as a new one.
		NyanType *inferred_type = obj->infer_type(astmember.name.get());

		// TODO: detect if two base classes define the same member
		//       independently

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
			if (inferred_type == nullptr) {
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
		nyan_op member_operation = nyan_op::INVALID;

		// only add a value if the member entry has one
		if (astmember.value.exists) {
			// requested operation for the value
			member_operation = astmember.operation;

			// check if the ast value can be combined with the member type
			this->check_member_value_type(member_type, member_operation, astmember.value);

			// create the member value
			member_value = this->create_member_value(astmember.value);
		}
		else {
			// no value given, the default constructor of nyanvalue
			// will do the job.
		}

		// create member
		obj->members.emplace(
			member_name,
			NyanMember{
				std::move(member_type),
				member_operation,
				std::move(member_value),
				NyanLocation{astmember.name}
			}
		);
	}

	return members;
}


void NyanParser::check_member_value_type(const NyanTypeContainer &member_type, nyan_op member_operation, const NyanASTMemberValue &astmembervalue) const {

	// TODO: also check the member_operation!

	// TODO: move this block to NyanValue and generalize
	// check if all the container values
	// are compatible with the container type
	size_t value_idx = 0;

	for (const NyanToken &value : astmembervalue.values) {
		NyanType value_type{value, *this->store};

		// multiple values given for non-container member
		if (value_idx > 1 and not member_type->is_container()) {
			throw TypeError{
				value,
				"storing multiple values in "
				"non-container member"
			};
		}

		// test if value (optionally in a container)
		// is compatible with the type required for the member
		if (member_type->is_container()) {
			if (not value_type.can_be_in(*member_type)) {
				throw TypeError{
					value,
					"value type incompatible to container type"
				};
			}
		}
		else {
			if (not value_type.is_child_of(*member_type)) {
				throw TypeError{
					value,
					"value type incompatible to member type"
				};
			}
		}

		value_idx += 1;
	}
}


NyanValueContainer NyanParser::create_member_value(const NyanASTMemberValue &astmembervalue) const {

	NyanValueContainer member_value;

	// switch by container type determined in the ast
	switch (astmembervalue.container_type) {
	case nyan_container_type::SINGLE: {
		const NyanToken &value_token = astmembervalue.values[0];
		nyan_type value_type = type_from_token(value_token);

		// create the NyanValue
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
			member_value = NyanValueContainer{
				this->store->get(value_token.get())
			};
			break;
		}
		default:
			throw NyanInternalError{"non-implemented value type"};
		}
		break;
	}
	case nyan_container_type::SET: {
		// create a set from the value list
		member_value = NyanValueContainer{
			std::make_unique<NyanSet>(astmembervalue.values)
		};
		break;
	}
	case nyan_container_type::ORDEREDSET:
		throw NyanInternalError{"TODO create orderedset"};
		break;

	default:
		throw NyanInternalError{"unhandled container type"};
	}

	return member_value;
}


void NyanParser::add_patch_targets(NyanObject *obj, const NyanASTObject &astobj) const {

	// if patch targets were specified, store them in the __patch__ member
	if (astobj.targets.size() > 0) {
		const NyanMember *patchmember = obj->get_member_ptr("__patch__");
		if (patchmember != nullptr) {
			throw NyanFileError{
				patchmember->get_location(),
				"explicit __patch__ member encountered "
				"when using <target> notation"
			};
		}

		// create the __patch__ member to contain the targets.
		// it's an orderedset of nyan objects.
		NyanMember patch_member{
			NyanTypeContainer{std::make_unique<NyanType>(
				nyan_container_type::ORDEREDSET,
				std::make_unique<NyanType>(nullptr)
				// ^ = nullptr object, so any NyanObject is allowed.
			)},
			NyanLocation{astobj.targets[0]}
		};

		// create the orderedset of patch target objects
		auto patch_targets = std::make_unique<NyanOrderedSet>();

		// add each patch target to the __patch__ member
		for (auto &target_name : astobj.targets) {
			NyanObject *tobj = this->store->get(target_name.get());
			if (tobj == nullptr) {
				throw NyanFileError{
					target_name,
					"unknown patch target object"
				};
			}
			patch_targets->add(NyanValueContainer{tobj});
		}

		// set the set as member value
		patch_member.set_value(std::move(patch_targets));

		// create the member entry
		obj->members.emplace("__patch__", std::move(patch_member));
	}
}


void NyanParser::inheritance_mod(NyanObject *obj, const NyanASTObject &astobj) const {
	for (auto &parent_name : astobj.inheritance_add) {
		// TODO: inheritance modifications as __parents_add__
		throw NyanInternalError{"TODO inheritance mod"};
	}
}

} // namespace nyan
