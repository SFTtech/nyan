// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_orderedset.h"

namespace nyan {

NyanOrderedSet::NyanOrderedSet() {}


NyanOrderedSet::NyanOrderedSet(const std::vector<NyanToken> &tokens) {
	throw NyanInternalError{"TODO"};
}


std::unique_ptr<NyanValue> NyanOrderedSet::copy() const {
	throw NyanInternalError{"TODO"};
}


std::string NyanOrderedSet::str() const {
	std::ostringstream builder;
	builder << "<";

	size_t cnt = 0;
	for (auto &value : this->values) {
		if (cnt > 0) {
			builder << ", ";
		}
		builder << value->str();
		cnt += 1;
	}

	builder << ">";
	return builder.str();
}


size_t NyanOrderedSet::hash() const {
	throw NyanInternalError{"TODO"};
}


bool NyanOrderedSet::contains(const NyanValue &value) {
	throw NyanInternalError{"TODO"};
}


void NyanOrderedSet::add(NyanValueContainer &&value) {
	throw NyanInternalError{"TODO"};
}


void NyanOrderedSet::add(const NyanValueContainer &value) {
	throw NyanInternalError{"TODO"};
}


void NyanOrderedSet::remove(const NyanValueContainer &value) {
	throw NyanInternalError{"TODO"};
}


void NyanOrderedSet::apply_value(const NyanValue *value, nyan_op operation) {
	throw NyanInternalError{"TODO"};
}


bool NyanOrderedSet::equals(const NyanValue &other) const {
	throw NyanInternalError{"TODO"};
}


const std::unordered_set<nyan_op> &NyanOrderedSet::allowed_operations() const {
	throw NyanInternalError{"TODO"};
}



} // namespace nyan
