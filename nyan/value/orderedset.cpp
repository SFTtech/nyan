// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "orderedset.h"

#include "../error.h"
#include "../util.h"


namespace nyan {

OrderedSet::OrderedSet() = default;


OrderedSet::OrderedSet(std::vector<ValueHolder> &&values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


ValueHolder OrderedSet::copy() const {
	return {std::make_shared<OrderedSet>(*this)};
}


bool OrderedSet::add(const ValueHolder &value) {
	return this->values.insert(value);
}


bool OrderedSet::contains(const ValueHolder &value) const {
	return this->values.contains(value);
}


bool OrderedSet::remove(const ValueHolder &value) {
	return (this->values.erase(value) == 1);
}


std::string OrderedSet::str() const {
	std::ostringstream builder;
	builder << "o{";
	builder << util::strjoin(
		", ", this->values,
		[] (const auto &val) {
			return val->str();
		}
	);
	builder << "}";

	return builder.str();
}


std::string OrderedSet::repr() const {
	std::ostringstream builder;
	builder << "o{";
	builder << util::strjoin(
		", ", this->values,
		[] (const auto &val) {
			return val->repr();
		}
	);
	builder << "}";
	return builder.str();
}


const std::unordered_set<nyan_op> &OrderedSet::allowed_operations(const Type &with_type) const {

	const static std::unordered_set<nyan_op> orderedset_ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	const static std::unordered_set<nyan_op> set_ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	if (not with_type.is_container()) {
		return no_nyan_ops;
	}

	switch (with_type.get_composite_type()) {
	case composite_t::ORDEREDSET:
		return orderedset_ops;

	case composite_t::SET:
		return set_ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &OrderedSet::get_type() const {
	constexpr static BasicType type{
		primitive_t::CONTAINER,
		composite_t::ORDEREDSET,
	};

	return type;
}

} // namespace nyan
