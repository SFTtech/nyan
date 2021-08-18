// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "object.h"

#include <unordered_set>
#include <vector>

#include "c3.h"
#include "compiler.h"
#include "database.h"
#include "error.h"
#include "object_info.h"
#include "object_state.h"
#include "patch_info.h"
#include "util.h"
#include "value/boolean.h"
#include "value/file.h"
#include "value/number.h"
#include "value/object.h"
#include "value/orderedset.h"
#include "value/set.h"
#include "value/text.h"
#include "view.h"


namespace nyan {

Object::Object(const fqon_t &name, const std::shared_ptr<View> &origin)
	:
	origin{origin},
	name{name} {}


Object::~Object() = default;


const fqon_t &Object::get_name() const {
	return this->name;
}


const std::shared_ptr<View> &Object::get_view() const {
	return this->origin;
}


ValueHolder Object::get_value(const memberid_t &member, order_t t) const {
	return this->calculate_value(member, t);
}


value_int_t Object::get_int(const memberid_t &member, order_t t) const {
	return this->get_number<Int>(member, t);
}


value_float_t Object::get_float(const memberid_t &member, order_t t) const {
	return this->get_number<Float>(member, t);
}


const std::string &Object::get_text(const memberid_t &member, order_t t) const {
	return *this->get<Text>(member, t);
}


bool Object::get_bool(const memberid_t &member, order_t t) const {
	return *this->get<Boolean>(member, t);
}


const set_t &Object::get_set(const memberid_t &member, order_t t) const {
	return this->get<Set>(member, t)->get();
}


const ordered_set_t &Object::get_orderedset(const memberid_t &member, order_t t) const {
	return this->get<OrderedSet>(member, t)->get();
}


const std::string &Object::get_file(const memberid_t &member, order_t t) const {
	return this->get<Filename>(member, t)->get();
}


Object Object::get_object(const memberid_t &member, order_t t) const {
	return *this->get<Object>(member, t);
}


template <>
std::shared_ptr<Object> Object::get<Object>(const memberid_t &member, order_t t) const {
	auto obj_val = this->get<ObjectValue>(member, t);

	const fqon_t &fqon = obj_val->get_name();
	std::shared_ptr<Object> ret = std::make_shared<Object>(
		Object::Restricted{},
		fqon,
		this->origin
	);
	return ret;
}


template <>
std::optional<std::shared_ptr<Object>> Object::get_optional<Object>(const memberid_t &member, order_t t) const {
	auto optional_obj_val = this->get_optional<ObjectValue>(member, t);
	if (not optional_obj_val.has_value()) {
		return {};
	}
	std::shared_ptr<ObjectValue> obj_val = std::move(optional_obj_val).value();

	const fqon_t &fqon = obj_val->get_name();
	std::shared_ptr<Object> ret = std::make_shared<Object>(
		Object::Restricted{},
		fqon, this->origin
	);
	return ret;
}


ValueHolder Object::calculate_value(const memberid_t &member, order_t t) const {
	using namespace std::string_literals;

	// TODO: don't allow calculating values for patches?
	// it's impossible as they may have members without =

	// get references to all parentobject-states
	std::vector<std::shared_ptr<ObjectState>> parents;

	const std::vector<fqon_t> &linearization = this->get_linearized(t);

	// find the last value assigning with =
	// it sets the base value where we apply the modifications then
	size_t defined_by = 0;

	const Value *base_value = nullptr;
	for (auto &obj : linearization) {
		parents.push_back(this->origin->get_raw(obj, t));
		const ObjectState *obj_raw = parents.back().get();
		const Member *obj_member = obj_raw->get(member);
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
	// errors in the data files are detected at load time already.
	if (unlikely(defined_by >= linearization.size() or base_value == nullptr)) {
		throw MemberNotFoundError{this->name, member};
	}

	// if this object defines the value, no aggregation is needed.
	if (defined_by == 0) {
		return base_value->copy();
	}

	// create a working copy of the value
	ValueHolder result = base_value->copy();

	// walk back and apply the value changes
	while (true) {
		const Member *change = parents[defined_by]->get(member);
		if (change != nullptr) {
			result->apply(*change);
		}
		if (defined_by == 0) {
			break;
		}
		defined_by -= 1;
	}

	return result;
}


const std::deque<fqon_t> &Object::get_parents(order_t t) const {
	return this->get_raw(t)->get_parents();
}


bool Object::has(const memberid_t &member, order_t t) const {
	// TODO: cache?

	const std::vector<fqon_t> &lin = this->get_linearized(t);

	for (auto &obj : lin) {
		if (this->origin->get_raw(obj, t)->get(member) != nullptr) {
			return true;
		}
	}

	return false;
}


bool Object::extends(fqon_t other_fqon, order_t t) const {
	if (this->name == other_fqon) {
		return true;
	}

	// TODO cache?

	auto &linearization = this->get_linearized(t);

	for (auto &obj : linearization) {
		if (obj == other_fqon) {
			return true;
		}
	}

	return false;
}


const ObjectInfo &Object::get_info() const {
	if (unlikely(not this->name.size())) {
		throw InvalidObjectError{};
	}

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


const fqon_t *Object::get_target() const {
	const PatchInfo *patch_info = this->get_info().get_patch().get();
	if (unlikely(patch_info == nullptr)) {
		return nullptr;
	}
	return &patch_info->get_target();
}


const std::vector<fqon_t> &Object::get_linearized(order_t t) const {
	if (unlikely(not this->name.size())) {
		throw InvalidObjectError{};
	}

	return this->origin->get_linearization(this->name, t);
}

std::shared_ptr<ObjectNotifier>
Object::subscribe(const update_cb_t &callback) {
	if (unlikely(not this->name.size())) {
		throw InvalidObjectError{};
	}

	return this->origin->create_notifier(this->name, callback);
}


const std::shared_ptr<ObjectState> &Object::get_raw(order_t t) const {
	if (unlikely(not this->name.size())) {
		throw InvalidObjectError{};
	}

	return this->origin->get_raw(this->name, t);
}

} // namespace nyan
