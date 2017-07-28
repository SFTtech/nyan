// Copyright 2017-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "state.h"

#include <sstream>

#include "error.h"
#include "object_state.h"
#include "view.h"
#include "util.h"


namespace nyan {

State::State(const std::shared_ptr<State> &previous_state)
	:
	previous_state{previous_state} {}


State::State()
	:
	previous_state{nullptr} {}


const std::shared_ptr<ObjectState> &State::get(const fqon_t &fqon) const {
	auto it = this->objects.find(fqon);
	if (it != std::end(this->objects)) {
		return it->second;
	}
	else {
		if (previous_state == nullptr) {
			throw Error{"unknown object requested"};
		}
		else {
			// search backwards.
			// TODO: optimize away with the last-changed map :)
			return previous_state->get(fqon);
		}
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


ObjectState &State::copy_object(const fqon_t &name, order_t t, std::shared_ptr<View> &origin) {

	std::shared_ptr<ObjectState> source = origin->get_raw(name, t);

	if (not source) {
		throw InternalError{"object copy source not found"};
	}

	// check if the object already is in this state
	auto it = this->objects.find(name);
	if (it == std::end(this->objects)) {
		// if not, copy the source object
		return *this->objects.emplace(
			name,
			source->copy()
		).first->second.get();
	}
	else {
		// if yes, check if they are the same already
		if (it->second != source) {
			// copy object to this state
			it->second = source->copy();
		}
		// else, no need to copy, the source would be this state anyway.
		return *it->second.get();
	}
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
