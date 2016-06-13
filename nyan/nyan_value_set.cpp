// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_set.h"

#include "nyan_error.h"
#include "nyan_ops.h"
#include "nyan_token.h"

#include "nyan_value_container.h"


namespace nyan {

NyanSet::NyanSet() {}


NyanSet::NyanSet(std::vector<NyanValueContainer> &values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


NyanSet::NyanSet(const NyanSet &other) {
	throw NyanInternalError{"TODO"};
}


std::unique_ptr<NyanValue> NyanSet::copy() const {
	return std::make_unique<NyanSet>(*this);
}


std::string NyanSet::str() const {
	std::ostringstream builder;
	builder << "{";

	size_t cnt = 0;
	for (auto &value : this->values) {
		if (cnt > 0) {
			builder << ", ";
		}
		builder << value->str();
		cnt += 1;
	}

	builder << "}";
	return builder.str();
}


std::string NyanSet::repr() const {
	std::ostringstream builder;
	builder << "{";

	size_t cnt = 0;
	for (auto &value : this->values) {
		if (cnt > 0) {
			builder << ", ";
		}
		builder << value->repr();
		cnt += 1;
	}

	builder << "}";
	return builder.str();
}


size_t NyanSet::hash() const {
	throw NyanError{"NyanSet is not hashable."};
}


bool NyanSet::add(NyanValueContainer &&value) {
	return std::get<1>(this->values.insert(std::move(value)));
}


bool NyanSet::contains(NyanValue *value) {
	return (this->values.find(value) != std::end(this->values));
}


bool NyanSet::remove(NyanValue *value) {
	return (1 == this->values.erase(value));
}


void NyanSet::apply_value(const NyanValue *value, nyan_op operation) {
	const NyanSet *change = dynamic_cast<const NyanSet *>(value);

	throw NyanInternalError{"TODO"};

	switch (operation) {
	case nyan_op::ASSIGN:
		break;

	case nyan_op::ADD_ASSIGN:
	case nyan_op::UNION_ASSIGN:
		break;

	case nyan_op::SUBTRACT_ASSIGN:
		break;

	case nyan_op::MULTIPLY_ASSIGN:
		break;

	case nyan_op::INTERSECT_ASSIGN:
		break;

	default:
		throw NyanError{"unknown operation requested"};
	}
}


/* test if the same values are in there */
bool NyanSet::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanSet &>(other);
	throw NyanInternalError{"TODO set equality"};
}


const std::unordered_set<nyan_op> &NyanSet::allowed_operations(nyan_type value_type) const {

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	// TODO: check for container type.

	if (value_type != nyan_type::CONTAINER) {
		return no_nyan_ops;
	}

	return ops;
}

} // namespace nyan
