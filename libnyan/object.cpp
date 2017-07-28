// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object.h"

#include <unordered_set>
#include <vector>

#include "c3.h"
#include "database.h"
#include "error.h"
#include "object_info.h"
#include "object_state.h"
#include "patch_info.h"
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


const Value &Object::get(const memberid_t &member, order_t t) {
	// TODO: don't allow for patches?
	// it's impossible as they may have members without =

	// TODO: try the cache lookup. but when is it invalid?

	// get references to all parentobject-states
	std::vector<std::shared_ptr<ObjectState>> parents;

	const std::vector<fqon_t> linearization = this->linearize_parents(t);

	// find the last value assigning with =
	// it sets the base value where we apply the modifications then
	size_t defined_by = 0;
	const Value *base_value = nullptr;
	for (auto &obj : linearization) {
		parents.push_back(this->origin->get_raw(obj, t));
		const ObjectState *obj_raw = parents.back().get();
		const Member *obj_member = obj_raw->get_member(member);
		// if the object has the member, check if it's the =
		if (obj_member != nullptr) {
			if (obj_member->get_operation() == nyan_op::ASSIGN) {
				base_value = &obj_member->get_value();
				break;
			}
		}
		defined_by += 1;
	}

	// no operator = was found for this member
	// -> no parent assigned a value.
	// This is normally detected at load time, but:
	// TODO: detect @-overrides that purge a = at load time!
	if (defined_by >= linearization.size() or base_value == nullptr) {
		throw InternalError{"no operator = found for member"};
	}

	// if this object defines the value, no aggregation is needed.
	if (defined_by == 0) {
		return *base_value;
	}

	// Create a working copy of the value
	ValueHolder result = base_value->copy();

	// walk back and apply the value changes
	while (true) {
		const Member *change = parents[defined_by]->get_member(member);
		if (change != nullptr) {
			result->apply(*change);
		}
		if (defined_by == 0) {
			break;
		}
		defined_by -= 1;
	}

	// Remember the data ptr.
	const Value &result_ref = *result.get_value();

	// Move result to cache, the reference will stay intact.
	parents[0]->get_member(member)->cache_save(std::move(result));

	// And return the reference.
	return result_ref;
}


const std::vector<fqon_t> &Object::get_parents(order_t t) {
	return this->origin->get_raw(this->get_name(), t)->get_parents();
}


bool Object::has(const memberid_t &member, order_t t) {
	const std::vector<fqon_t> &lin = this->linearize_parents(t);

	for (auto &obj : lin) {
		if (this->origin->get_raw(obj, t)->get_member(member) != nullptr) {
			return true;
		}
	}

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


const ObjectInfo &Object::get_info() const {
	const ObjectInfo *ret = this->origin->get_database().get_info().get_object(this->get_name());
	if (unlikely(ret == nullptr)) {
		throw InternalError{"object info unavailable for object handle"};
	}
	return *ret;
}



bool Object::is_patch() const {
	// must be a patch from the beginning of time!
	return this->get_info().is_patch();
}


const fqon_t &Object::get_target() const {
	const PatchInfo *patch_info = this->get_info().get_patch().get();
	if (unlikely(patch_info == nullptr)) {
		throw InternalError{"queried target on non-patch"};
	}
	return patch_info->get_target();
}


const std::vector<fqon_t> &Object::linearize_parents(order_t t) {
	return linearize(
		this->name,
		[this, &t] (const fqon_t &name) -> ObjectState& {
			ObjectState *state = this->origin->get_raw(name, t).get();
			if (unlikely(state == nullptr)) {
				throw InternalError{"object state not found for parent"};
			}
			return *state;
		}
	);
}


std::shared_ptr<ObjectState> Object::get_raw(order_t t) const {
	return this->origin->get_raw(this->name, t);
}

} // namespace nyan
