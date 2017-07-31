// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "orderedset.h"

#include "error.h"


namespace nyan {

OrderedSet::OrderedSet() {}


OrderedSet::OrderedSet(std::vector<ValueHolder> &&values) {
	for (auto &value : values) {
		this->values.add(std::move(value));
	}
}


ValueHolder OrderedSet::copy() const {
	return ValueHolder{
		std::make_shared<OrderedSet>(dynamic_cast<const OrderedSet &>(*this))
	};
}


bool OrderedSet::add(const ValueHolder &value) {
	return this->values.add(value);
}


bool OrderedSet::contains(const ValueHolder &value) {
	return this->values.contains(value);
}


bool OrderedSet::remove(const ValueHolder &value) {
	throw InternalError{"TODO ordered set remove"};
}


void OrderedSet::apply_value(const Value &value, nyan_op operation) {
	throw InternalError{"TODO orderedset apply value"};

	// the other value may be a generic set
	const OrderedSet &change = dynamic_cast<const OrderedSet &>(value);

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


const std::unordered_set<nyan_op> &OrderedSet::allowed_operations(const Type &with_type) const {

	if (not with_type.is_container()) {
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

	switch (with_type.get_container_type()) {
	case container_t::SET:
		return set_ops;

	case container_t::ORDEREDSET:
		return orderedset_ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &OrderedSet::get_type() const {
	constexpr static BasicType type{
		primitive_t::CONTAINER,
		container_t::ORDEREDSET,
	};

	return type;
}

} // namespace nyan
