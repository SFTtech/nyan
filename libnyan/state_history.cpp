// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "state_history.h"

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


const State &StateHistory::get_state(order_t t) const {
	const State *state = this->history.at(t).get();
	if (unlikely(state == nullptr)) {
		throw InternalError{"curve returned nullptr state"};
	}

	return *state;
}


const std::shared_ptr<State> &StateHistory::get_state_ptr(order_t t) const {
	return this->history.at(t);
}


const std::shared_ptr<State> &StateHistory::get_state_before(order_t t) const {
	return this->history.at(t)->get_previous_state();
}


const std::shared_ptr<State> *StateHistory::get_state_exact(order_t t) const {
	return this->history.at_exact(t);
}


void StateHistory::insert(std::shared_ptr<State> &&new_state, order_t t) {
	// drop all later states
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
