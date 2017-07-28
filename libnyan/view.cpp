// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "view.h"

#include "database.h"
#include "object_state.h"
#include "state.h"


namespace nyan {


View::View(const std::shared_ptr<Database> &database)
	:
	database{database} {

	// insert a new state at beginning of time
	this->history.insert_drop(
		DEFAULT_T,
		std::make_shared<State>(this->database->get_state())
	);
}


Object View::get(fqon_t fqon) {
	return Object{fqon, shared_from_this()};
}


const std::shared_ptr<ObjectState> &View::get_raw(fqon_t fqon, order_t t) {
	return this->get_state(t).get(fqon);
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
	// TODO optimize: only search once.
	// currently searches once for at_exact and once for at

	// this state is already existing
	std::shared_ptr<State> *state = this->history.at_exact(t);
	// check if there is a state at t
	if (state != nullptr) {
		return **state;
	}

	// use the previous state at the next lower t
	std::shared_ptr<State> new_state = std::make_shared<State>(
		this->history.at(t)
	);

	// drop all later states
	return *this->history.insert_drop(t, std::move(new_state)).get();
}


const State &View::get_state(order_t t) {
	if (this->history.empty()) {
		return this->get_database().get_state();
	}

	const State *state = this->history.at(t).get();
	if (unlikely(state == nullptr)) {
		throw InternalError{"curve returned nullptr state"};
	}

	return *state;
}


const Database &View::get_database() const {
	return *this->database;
}


void View::add_child(const std::shared_ptr<View> &view) {
	this->children.push_back(view);
}

} // namespace nyan
