// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "view.h"

#include "c3.h"
#include "database.h"
#include "object_state.h"
#include "state.h"


namespace nyan {


View::View(const std::shared_ptr<Database> &database)
	:
	database{database},
	state{database} {}


Object View::get(const fqon_t &fqon) {
	// test for object existence
	this->get_info(fqon);

	return Object{fqon, shared_from_this()};
}


const std::shared_ptr<ObjectState> &View::get_raw(const fqon_t &fqon, order_t t) {
	// TODO optimize: speed up the backtrack search!
	return this->get_state(t).get_search(fqon);
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


std::vector<std::weak_ptr<View>> &View::get_children() {
	return this->children;
}


const State &View::get_state(order_t t) const {
	return this->state.get_state(t);
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
	this->children.push_back(view);
}


} // namespace nyan
