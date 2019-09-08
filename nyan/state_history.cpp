// Copyright 2017-2019 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "state_history.h"

#include "compiler.h"
#include "database.h"
#include "meta_info.h"
#include "state.h"


namespace nyan {

StateHistory::StateHistory(const std::shared_ptr<Database> &base) {

	// create new empty state to work on at the beginning.
	this->insert(
		std::make_shared<State>(base->get_state()),
		DEFAULT_T
	);
}

const std::shared_ptr<State> &StateHistory::get_state(order_t t) const {
	return this->history.at(t);
}


const std::shared_ptr<State> &StateHistory::get_state_before(order_t t) const {
	return this->history.at(t)->get_previous_state();
}


const std::shared_ptr<State> *StateHistory::get_state_exact(order_t t) const {
	return this->history.at_exact(t);
}


const std::shared_ptr<ObjectState> *StateHistory::get_obj_state(const fqon_t &fqon, order_t t) const {
	// get the object history
	const ObjectHistory *obj_history = this->get_obj_history(fqon);

	// object isn't recorded in this state history
	if (obj_history == nullptr) {
		return nullptr;
	}

	std::optional<order_t> order = obj_history->last_change_before(t);

	if (not order) {
		// the change is earlier than what is recorded in this history.
		return nullptr;
	}

	// now we know the time in history the object was changed
	const std::shared_ptr<State> *state = this->history.at_exact(*order);
	if (unlikely(state == nullptr)) {
		throw InternalError{"no history record at change point"};
	}

	const std::shared_ptr<ObjectState> *obj_state = (*state)->get(fqon);
	if (unlikely(state == nullptr)) {
		throw InternalError{"object state not found at change point"};
	}

	return obj_state;
}


void StateHistory::insert(std::shared_ptr<State> &&new_state, order_t t) {

	// record the changes.
	for (const auto &it : new_state->get_objects()) {
		ObjectHistory &obj_history = this->get_create_obj_history(it.first);
		obj_history.insert_change(t);
	}

	// drop all later changes
	this->history.insert_drop(t, std::move(new_state));
}


void StateHistory::insert_linearization(std::vector<fqon_t> &&ins, order_t t) {
	const auto &obj = ins.at(0);

	this->get_create_obj_history(obj).linearizations.insert_drop(t, std::move(ins));
}


const std::vector<fqon_t> &
StateHistory::get_linearization(const fqon_t &obj, order_t t,
                                const MetaInfo &meta_info) const {

	const ObjectHistory *obj_hist = this->get_obj_history(obj);
	if (obj_hist != nullptr) {
		if (not obj_hist->linearizations.empty()) {
			auto ret = obj_hist->linearizations.at_find(t);

			if (ret != nullptr) {
				return *ret;
			}
		}
	}

	// otherwise, the lin is only stored in the database.
	const ObjectInfo *obj_info = meta_info.get_object(obj);
	if (unlikely(obj_info == nullptr)) {
		throw InternalError{"object not found in metainfo"};
	}

	return obj_info->get_linearization();
}


void StateHistory::insert_children(const fqon_t &obj,
                                   std::unordered_set<fqon_t> &&ins,
                                   order_t t) {

	this->get_create_obj_history(obj).children.insert_drop(t, std::move(ins));
}


const std::unordered_set<fqon_t> &
StateHistory::get_children(const fqon_t &obj, order_t t,
                           const MetaInfo &meta_info) const {

	// first try the obj_history
	const ObjectHistory *obj_hist = this->get_obj_history(obj);
	if (obj_hist != nullptr) {
		if (not obj_hist->children.empty()) {
			auto ret = obj_hist->children.at_find(t);

			if (ret != nullptr) {
				return *ret;
			}
		}
	}

	// otherwise, the lin is only stored in the database.
	const ObjectInfo *obj_info = meta_info.get_object(obj);
	if (unlikely(obj_info == nullptr)) {
		throw InternalError{"object not found in metainfo"};
	}

	return obj_info->get_children();
}


ObjectHistory *StateHistory::get_obj_history(const fqon_t &obj) {
	auto it = this->object_obj_hists.find(obj);
	if (it != std::end(this->object_obj_hists)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


const ObjectHistory *StateHistory::get_obj_history(const fqon_t &obj) const {
	auto it = this->object_obj_hists.find(obj);
	if (it != std::end(this->object_obj_hists)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


ObjectHistory &StateHistory::get_create_obj_history(const fqon_t &obj) {
	auto it = this->object_obj_hists.find(obj);
	if (it != std::end(this->object_obj_hists)) {
		return it->second;
	}
	else {
		// create new obj_history entry.
		return this->object_obj_hists.emplace(
			obj, ObjectHistory{}
		).first->second;
	}
}

} // namespace nyan
