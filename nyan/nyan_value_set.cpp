// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "nyan_value_set.h"

#include "nyan_error.h"


namespace nyan {

NyanSet::NyanSet() {}


NyanSet::NyanSet(std::vector<NyanValueContainer> &values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


std::unique_ptr<NyanValue> NyanSet::copy() const {
	throw NyanInternalError{"TODO set copy"};
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


const std::unordered_set<nyan_op> &NyanSet::allowed_operations(nyan_basic_type value_type) const {

	if (not value_type.is_container()) {
		return no_nyan_ops;
	}

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	switch (value_type.container_type) {
	case nyan_container_type::SET:
		return ops;

	default:
		return no_nyan_ops;
	}
}


const nyan_basic_type &NyanSet::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::CONTAINER,
		nyan_container_type::SET,
	};

	return type;
}

} // namespace nyan
