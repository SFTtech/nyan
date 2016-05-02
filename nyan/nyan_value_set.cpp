// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_set.h"

#include "nyan_error.h"
#include "nyan_ops.h"
#include "nyan_token.h"


namespace nyan {

NyanSet::NyanSet() {}


NyanSet::NyanSet(const std::vector<NyanToken> &tokens) {
	throw NyanInternalError{"TODO"};
}


NyanSet::NyanSet(const NyanSet &other) {
	throw NyanInternalError{"TODO"};
}


std::unique_ptr<NyanValue> NyanSet::copy() const {
	return std::make_unique<NyanSet>(dynamic_cast<const NyanSet &>(*this));
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


size_t NyanSet::hash() const {
	throw NyanInternalError{"TODO"};
}


bool NyanSet::contains(const NyanValue &value) {
	throw NyanInternalError{"TODO"};
}


void NyanSet::add(NyanValueContainer &&value) {
	throw NyanInternalError{"TODO"};
}


void NyanSet::add(const NyanValueContainer &value) {
	throw NyanInternalError{"TODO"};
}


void NyanSet::remove(const NyanValueContainer &value) {
	throw NyanInternalError{"TODO"};
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


bool NyanSet::equals(const NyanValue &other) const {
	auto &other_val = dynamic_cast<const NyanSet &>(other);
	throw NyanInternalError{"TODO"};
}


const std::unordered_set<nyan_op> &NyanSet::allowed_operations() const {
	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::MULTIPLY_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	return ops;
}

} // namespace nyan
