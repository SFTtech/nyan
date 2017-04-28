// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "set.h"

#include "error.h"


namespace nyan {

Set::Set() {}


Set::Set(std::vector<ValueContainer> &values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


std::unique_ptr<Value> Set::copy() const {
	throw InternalError{"TODO set copy"};
}


bool Set::add(ValueContainer &&value) {
	return std::get<1>(this->values.insert(std::move(value)));
}


bool Set::contains(Value *value) {
	return (this->values.find(value) != std::end(this->values));
}


bool Set::remove(Value *value) {
	return (1 == this->values.erase(value));
}


void Set::apply_value(const Value *value, nyan_op operation) {
	const Set *change = dynamic_cast<const Set *>(value);

	throw InternalError{"TODO"};

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
		throw Error{"unknown operation requested"};
	}
}


std::string Set::str() const {
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


std::string Set::repr() const {
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


const std::unordered_set<nyan_op> &Set::allowed_operations(nyan_basic_type value_type) const {

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


const nyan_basic_type &Set::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::CONTAINER,
		nyan_container_type::SET,
	};

	return type;
}

} // namespace nyan
