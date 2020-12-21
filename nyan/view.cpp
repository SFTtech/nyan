// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "view.h"

#include "c3.h"
#include "database.h"
#include "object_notifier.h"
#include "object_state.h"
#include "state.h"


namespace nyan {


View::View(const std::shared_ptr<Database> &database)
	:
	database{database},
	state{database} {}


Object View::get_object(const fqon_t &fqon) {
	// test for object existence
	this->get_info(fqon);

	// TODO: store info in object to avoid further lookups.
	return Object{fqon, shared_from_this()};
}


const std::shared_ptr<ObjectState> &View::get_raw(const fqon_t &fqon, order_t t) const {
	auto state = this->state.get_obj_state(fqon, t);
	if (state == nullptr) {
		auto &dbstate = this->database->get_state();
		auto db_obj_state = dbstate->get(fqon);
		if (db_obj_state == nullptr) {
			throw ObjectNotFoundError{fqon};
		}

		return *db_obj_state;
	}
	else {
		return *state;
	}
}


const ObjectInfo &View::get_info(const fqon_t &fqon) const {
	const ObjectInfo *info = this->database->get_info().get_object(fqon);
	if (unlikely(info == nullptr)) {
		throw ObjectNotFoundError{fqon};
	}

	return *info;
}


Transaction View::new_transaction(order_t t) {
	return Transaction{t, shared_from_this()};
}


std::shared_ptr<View> View::new_child() {
	auto ret = std::make_shared<View>(this->database);
	this->add_child(ret);
	return ret;
}


void View::cleanup_stale_children() {
	auto it = std::begin(this->children);

	while (it != std::end(this->children)) {
		if (it->expired()) {
			if (this->children.size() > 1) {
				std::iter_swap(it, std::end(this->children) - 1);
				this->children.pop_back();
			}
			else {
				this->children.clear();
			}
		}
		else {
			++it;
		}
	}
}


const std::vector<std::weak_ptr<View>> &View::get_children() {
	return this->children;
}


const Database &View::get_database() const {
	return *this->database;
}


const std::vector<fqon_t> &View::get_linearization(const fqon_t &fqon, order_t t) const {
	return this->state.get_linearization(fqon, t, this->get_database().get_info());
}


const std::unordered_set<fqon_t> &View::get_obj_children(const fqon_t &fqon, order_t t) const {
	return this->state.get_children(fqon, t, this->get_database().get_info());
}


std::unordered_set<fqon_t> View::get_obj_children_all(const fqon_t &fqon, order_t t) const {
	std::unordered_set<fqon_t> ret;

	this->gather_obj_children(ret, fqon, t);

	return ret;
}


std::shared_ptr<ObjectNotifier> View::create_notifier(const fqon_t &fqon,
                                                      const update_cb_t &callback) {

	auto it = this->notifiers.find(fqon);
	decltype(this->notifiers)::mapped_type *notifier_set = nullptr;

	if (it == std::end(this->notifiers)) {

		// create new set, add to object map and and get pointer
		auto ins = this->notifiers.insert(
			{
				fqon,
				std::unordered_set<std::shared_ptr<ObjectNotifierHandle>>{},
			}
		);

		notifier_set = &ins.first->second;
	}
	else {
		notifier_set = &it->second;
	}

	auto notifier = std::make_shared<ObjectNotifier>(fqon, callback, this->shared_from_this());
	const auto& handle = notifier->get_handle();
	notifier_set->insert(handle);
	return notifier;
}


void View::deregister_notifier(const fqon_t &fqon,
                               const std::shared_ptr<ObjectNotifierHandle> &notifier) {
	auto it = this->notifiers.find(fqon);
	if (it != std::end(this->notifiers)) {
		size_t removed = it->second.erase(notifier);
		if (removed == 0) {
			throw InternalError{"could not find notifier instance in fqon set to deregister"};
		}
	}
	else {
		throw InternalError{"could not find notifier set by fqon to deregister"};
	}
}


void View::fire_notifications(const std::unordered_set<fqon_t> &changed_objs,
                              order_t t) const {
	for (auto &obj : changed_objs) {
		auto it = this->notifiers.find(obj);
		if (it != std::end(this->notifiers)) {
			for (auto &notifier : it->second) {
				const std::shared_ptr<ObjectState> &obj_state = this->get_raw(obj, t);
				notifier->fire(t, obj, *obj_state);
			}
		}
	}
}



void View::gather_obj_children(std::unordered_set<fqon_t> &target,
                               const fqon_t &obj,
                               order_t t) const {

	for (auto &child : this->get_obj_children(obj, t)) {
		target.insert(child);

		// TODO optimize: maybe it's faster to not recurse
		//                if that child was already in the set.
		//                because then all its childs will
		//                be in there already.
		this->gather_obj_children(target, child, t);
	}
}



StateHistory &View::get_state_history() {
	return this->state;
}


void View::add_child(const std::shared_ptr<View> &view) {
	view->parent_view = this->shared_from_this();
	this->children.push_back(view);
}


} // namespace nyan
