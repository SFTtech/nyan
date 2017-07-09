// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object.h"

#include <unordered_set>
#include <vector>

#include "c3.h"
#include "database.h"
#include "error.h"
#include "object_state.h"
#include "util.h"
#include "value/object.h"
#include "view.h"


namespace nyan {

Object::Object(const fqon_t &name, const std::shared_ptr<View> &origin)
	:
	origin{origin},
	name{name} {}


Object::~Object() {}


const fqon_t &Object::get_name() const {
	return this->name;
}


const View &Object::get_view() const {
	return *this->origin;
}


const Value &Object::get(const memberid_t &member, order_t t) const {
	// TODO: don't allow for patches? only possible if there's at least one = for that member

	// TODO: try the cache lookup. but when is it invalid?

	/*
	// get references to all parentobject-states
	std::vector<std::shared_ptr<ObjectState>> parents;
	for (auto &parent : this->linearize_parents(t)) {
		parents.push_back(this->origin->get_raw(parent, t));
	}

	// find the last value assigning with =
	// it sets the base value where we apply the modifications then
	size_t defined_by = 0;
	Value *base_value = nullptr;
	for (auto &obj : parents) {
		const Member *obj_member = obj->get_member(member);
		if (obj_member != nullptr) {
			if (obj_member->get_operation() == nyan_op::ASSIGN) {
				base_value = obj_member->get_value_ptr();
				break;
			}
		}
		defined_by += 1;
	}

	// no operator = was found for this member
	// -> no parent assigned a value.
	// TODO: detect this at load time!
	if (defined_by >= linearization.size() or base_value == nullptr) {
		throw Error{"no operator = found for member"};
	}

	// if this object defines the value, no aggregation is needed.
	if (defined_by == 0) {
		return *base_value;
	}

	// Create a working copy of the value
	// TODO: store it in the first parent, i.e. this object state
	std::unique_ptr<Value> result = base_value->copy();

	// walk back and apply the value changes
	while (true) {
		// asdf member lookup
		const Member *change = parents[defined_by]->get_member_ptr(member);
		if (change != nullptr) {
			result->apply(change);
		}
		if (defined_by == 0) {
			break;
		}
		defined_by -= 1;
	}

	// Remember the data ptr.
	const Value &result_ref = *result.get();

	// Move result to cache,
	obj_member->cache_save(std::move(result));

	// and return the reference.
	return result_ref;
	*/

}


const std::vector<fqon_t> &Object::get_parents(order_t t) {
	// asdf look in view for parents
}


const Type &Object::get_member_type(const std::string &member) const {
	// asdf contact type system
}


bool Object::has(const memberid_t &member, order_t t) const {
	// asdf
	return false;
}


bool Object::extends(fqon_t other_fqon, order_t t) {
	if (this->name == other_fqon) {
		return true;
	}

	auto &linearization = this->linearize_parents(t);

	for (auto &obj : linearization) {
		if (obj == other_fqon) {
			return true;
		}
	}

	return false;
}



bool Object::is_patch() const {
	// must be a patch from the beginning of time!
	// TODO: typedb knows if patch, no need to process
	return this->has("__patch__", DEFAULT_T);
}


const fqon_t &Object::get_target() const {
	return this->get<ObjectValue>("__patch__", DEFAULT_T).get();
}


const std::vector<fqon_t> &Object::linearize_parents(order_t t) {
	return linearize(
		this->name,
		[this, &t] (const fqon_t &name) -> ObjectState& {
			return *this->origin->get_raw(name, t);
		}
	);
}


std::shared_ptr<ObjectState> Object::get_raw(order_t t) const {
	return this->origin->get_raw(this->name, t);
}

} // namespace nyan
