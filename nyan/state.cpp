// Copyright 2017-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "state.h"

#include <sstream>

#include "compiler.h"
#include "error.h"
#include "object_state.h"
#include "util.h"
#include "view.h"


namespace nyan {

State::State(const std::shared_ptr<State> &previous_state) :
	previous_state{previous_state} {}


State::State() :
	previous_state{nullptr} {}


const std::shared_ptr<ObjectState> *State::get(const fqon_t &fqon) const {
	auto it = this->objects.find(fqon);
	if (it != std::end(this->objects)) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}


ObjectState &State::add_object(const fqon_t &name, std::shared_ptr<ObjectState> &&obj) {
	if (unlikely(this->previous_state != nullptr)) {
		throw InternalError{"can't add new object in state that is not initial."};
	}

	auto ins = this->objects.insert({name, std::move(obj)});

	if (not ins.second) {
		throw InternalError{"added an already-known object to the state!"};
	}

	return *ins.first->second;
}


void State::update(std::shared_ptr<State> &&source_state) {
	// update this state with all objects from the source state
	// -> move all objects from the sourcestate into this one.
	for (auto &it : source_state.get()->objects) {
		auto search = this->objects.find(it.first);
		if (search != std::end(this->objects)) {
			search->second = std::move(it.second);
		}
		else {
			this->objects.insert(
				{std::move(it.first), std::move(it.second)});
		}
	}
}


const std::shared_ptr<ObjectState> &State::copy_object(const fqon_t &name,
                                                       order_t t,
                                                       std::shared_ptr<View> &origin) {
	// last known state of the object
	const std::shared_ptr<ObjectState> &source = origin->get_raw(name, t);

	if (not source) {
		throw InternalError{"object copy source not found"};
	}

	// check if the object already is in this state
	auto it = this->objects.find(name);
	if (it == std::end(this->objects)) {
		// if not, copy the source object into this state
		auto it_new_object = this->objects.emplace(name, source->copy()).first;
		return it_new_object->second;
	}
	else {
		// else, no need to copy, the object is already in this state
		return it->second;
	}
}


const std::shared_ptr<State> &State::get_previous_state() const {
	return this->previous_state;
}


const std::unordered_map<fqon_t, std::shared_ptr<ObjectState>> &
State::get_objects() const {
	return this->objects;
}


std::string State::str() const {
	std::ostringstream builder;

	builder << "State:" << std::endl;

	size_t i = 0;
	for (auto &it : this->objects) {
		builder << "object " << i << ":" << std::endl
				<< it.first << " => " << it.second->str() << std::endl;
		i += 1;
	}

	return builder.str();
}

} // namespace nyan
