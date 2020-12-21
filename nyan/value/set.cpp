// Copyright 2016-2021 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "set.h"

#include "../error.h"
#include "../util.h"


namespace nyan {

Set::Set() = default;


Set::Set(std::vector<ValueHolder> &&values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


ValueHolder Set::copy() const {
	return {std::make_shared<Set>(*this)};
}


bool Set::add(const ValueHolder &value) {
	return std::get<1>(this->values.insert(value));
}


bool Set::contains(const ValueHolder &value) const {
	return (this->values.find(value) != std::end(this->values));
}


bool Set::remove(const ValueHolder &value) {
	return (1 == this->values.erase(value));
}


std::string Set::str() const {
	// same as repr(), except we use str().

	std::ostringstream builder;
	builder << "{";
	builder << util::strjoin(
		", ", this->values,
		[] (const auto &val) {
			return val->str();
		}
	);
	builder << "}";

	return builder.str();
}


std::string Set::repr() const {
	// same as str(), except we use repr().

	std::ostringstream builder;
	builder << "{";
	builder << util::strjoin(
		", ", this->values,
		[] (const auto &val) {
			return val->repr();
		}
	);
	builder << "}";
	return builder.str();
}


const std::unordered_set<nyan_op> &Set::allowed_operations(const Type &with_type) const {

	const static std::unordered_set<nyan_op> none_ops{
		nyan_op::ASSIGN,
	};

	const static std::unordered_set<nyan_op> set_ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	const static std::unordered_set<nyan_op> orderedset_ops{
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	if (with_type.get_primitive_type() == primitive_t::NONE) {
		return none_ops;
	}

	if (not with_type.is_container()) {
		return no_nyan_ops;
	}

	switch (with_type.get_composite_type()) {
	case composite_t::SET:
		return set_ops;

	case composite_t::ORDEREDSET:
		return orderedset_ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &Set::get_type() const {
	constexpr static BasicType type{
		primitive_t::CONTAINER,
		composite_t::SET,
	};

	return type;
}

} // namespace nyan
