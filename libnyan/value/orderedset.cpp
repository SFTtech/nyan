// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "orderedset.h"

#include "error.h"


namespace nyan {

OrderedSet::OrderedSet() {}


OrderedSet::OrderedSet(std::vector<ValueContainer> &values) {
	for (auto &value : values) {
		this->values.add(std::move(value));
	}
}


std::unique_ptr<Value> OrderedSet::copy() const {
	throw InternalError{"TODO ordered set copy"};
}


bool OrderedSet::add(ValueContainer &&value) {
	return this->values.add(std::move(value));
}


bool OrderedSet::contains(Value *value) {
	return this->values.contains(value);
}


bool OrderedSet::remove(Value *value) {
	throw InternalError{"TODO ordered set remove"};
}


void OrderedSet::apply_value(const Value *value, nyan_op operation) {
	const OrderedSet *change = dynamic_cast<const OrderedSet *>(value);

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


std::string OrderedSet::str() const {
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


std::string OrderedSet::repr() const {
	std::ostringstream builder;
	builder << "<";

	size_t cnt = 0;
	for (auto &value : this->values) {
		if (cnt > 0) {
			builder << ", ";
		}
		builder << value->repr();
		cnt += 1;
	}

	builder << ">";
	return builder.str();
}


const std::unordered_set<nyan_op> &OrderedSet::allowed_operations(nyan_basic_type value_type) const {

	if (not value_type.is_container()) {
		return no_nyan_ops;
	}

	const static std::unordered_set<nyan_op> set_ops{
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	const static std::unordered_set<nyan_op> orderedset_ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	switch (value_type.container_type) {
	case nyan_container_type::SET:
		return set_ops;

	case nyan_container_type::ORDEREDSET:
		return orderedset_ops;

	default:
		return no_nyan_ops;
	}
}


const nyan_basic_type &OrderedSet::get_type() const {
	constexpr static nyan_basic_type type{
		nyan_primitive_type::CONTAINER,
		nyan_container_type::ORDEREDSET,
	};

	return type;
}

} // namespace nyan
