// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "view.h"

#include "object_state.h"
#include "state.h"


namespace nyan {


View::View(const std::shared_ptr<Database> &database)
	:
	database{database} {}


Object View::get(fqon_t fqon) {
	return Object{fqon, shared_from_this()};
}


std::shared_ptr<ObjectState> View::get_raw(fqon_t fqon, order_t t) {
	return this->get_state(t)->get(fqon);
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


State &View::new_state(order_t t) {
	// TODO: asdf create a new state if not existing at exact t
	// previous state of it must be the next t lower that is known
	// drop all later states
	throw Error{"create new state"};
}


const std::shared_ptr<State> &View::get_state(order_t t) {
	return this->state.at(t);
}


void View::add_child(const std::shared_ptr<View> &view) {
	this->children.push_back(view);
}

} // namespace nyan
