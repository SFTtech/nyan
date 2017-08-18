// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "set.h"

#include "error.h"


namespace nyan {

Set::Set() {}


Set::Set(std::vector<ValueHolder> &&values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


ValueHolder Set::copy() const {
	return ValueHolder{
		std::make_shared<Set>(dynamic_cast<const Set &>(*this))
	};
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
	// same as str(), except we use repr().

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


const std::unordered_set<nyan_op> &Set::allowed_operations(const Type &with_type) const {

	if (not with_type.is_container()) {
		return no_nyan_ops;
	}

	const static std::unordered_set<nyan_op> ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	switch (with_type.get_container_type()) {
	case container_t::SET:
		return ops;

	default:
		return no_nyan_ops;
	}
}


const BasicType &Set::get_type() const {
	constexpr static BasicType type{
		primitive_t::CONTAINER,
		container_t::SET,
	};

	return type;
}

} // namespace nyan
