// Copyright 2020-2020 the nyan authors, LGPLv3+. See copying.md for legal info.

#include "dict.h"

#include "orderedset.h"
#include "set.h"
#include "../error.h"
#include "../util.h"


namespace nyan {

Dict::Dict() = default;

Dict::Dict(std::unordered_map<ValueHolder,ValueHolder> &&values) {
	for (auto &value : values) {
		this->values.insert(std::move(value));
	}
}


ValueHolder Dict::copy() const {
	// TODO
	return {std::make_shared<Dict>(*this)};
	// return ValueHolder();
}


std::string Dict::str() const {
	// same as repr(), except we use str().

	std::ostringstream builder;
	builder << "{";
	builder << util::strjoin(
		", ", this->values,
		[] (const auto &val) {
			return val.first->str() + ": " + val.second->str();
		}
	);
	builder << "}";

	return builder.str();
}


std::string Dict::repr() const {
	// same as str(), except we use repr().

	std::ostringstream builder;
	builder << "{";
	builder << util::strjoin(
		", ", this->values,
		[] (const auto &val) {
			return val.first->repr() + ": " + val.second->repr();
		}
	);
	builder << "}";

	return builder.str();
}


const BasicType &Dict::get_type() const {
	constexpr static BasicType type{
		primitive_t::CONTAINER,
		container_t::DICT,
	};

	return type;
}

void Dict::apply_value(const Value &value, nyan_op operation) {
	auto dict_applier = [](auto &member_value, auto operand, nyan_op operation) {
		switch (operation) {
		case nyan_op::ASSIGN:
			member_value.clear();
			// fall through

		// TODO
		case nyan_op::UNION_ASSIGN:
		case nyan_op::ADD_ASSIGN: {
			for (auto &val : operand) {
				member_value.insert(val);
			}
			break;
		}

		case nyan_op::INTERSECT_ASSIGN:{
			// only keep the values that are in both. Both key
			// and value must match.

			std::unordered_map<key_type, element_type> keep;
			keep.reserve(member_value.size());

			break;
		}

		default:
			throw InternalError{"unknown dict value application"};
		}
	};

	auto set_applier = [](auto &member_value, auto operand, nyan_op operation) {
		switch (operation) {
		case nyan_op::SUBTRACT_ASSIGN: {
			for (auto &val : operand) {
				member_value.erase(val);
			}
			break;
		}

		case nyan_op::INTERSECT_ASSIGN:
			// TODO
			break;

		default:
			throw InternalError{"unknown dict value application"};
		}
	};


	if (typeid(Set&) == typeid(value)) {
		const Set *change = dynamic_cast<const Set *>(&value);

		if (unlikely(change == nullptr)) {
			using namespace std::string_literals;
			throw InternalError{
				"set value application was not a container, it was: "s
				+ util::demangle(typeid(value).name())
				+ " and couldn't cast to "
				+ util::demangle(typeid(change).name())};
		}

		set_applier(this->values, change->get(), operation);

	}
	else if (typeid(OrderedSet&) == typeid(value)) {
		const OrderedSet *change = dynamic_cast<const OrderedSet *>(&value);

		if (unlikely(change == nullptr)) {
			using namespace std::string_literals;
			throw InternalError{
				"set value application was not a container, it was: "s
				+ util::demangle(typeid(value).name())
				+ " and couldn't cast to "
				+ util::demangle(typeid(change).name())};
		}

		set_applier(this->values, change->get(), operation);

	}
	else if (typeid(Dict&) == typeid(value)) {
		const Dict *change = dynamic_cast<const Dict *>(&value);

		if (unlikely(change == nullptr)) {
			using namespace std::string_literals;
			throw InternalError{
				"set value application was not a container, it was: "s
				+ util::demangle(typeid(value).name())
				+ " and couldn't cast to "
				+ util::demangle(typeid(change).name())};
		}

		dict_applier(this->values, change->get(), operation);
	}
	else {
		throw InternalError("expected Container instance for operation, but got"
					  		+ std::string(typeid(value).name()));
	}


}

const std::unordered_set<nyan_op> &Dict::allowed_operations(const Type &with_type) const {

	if (not with_type.is_container()) {
		return no_nyan_ops;
	}

	const static std::unordered_set<nyan_op> set_ops{
		nyan_op::SUBTRACT_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	const static std::unordered_set<nyan_op> dict_ops{
		nyan_op::ASSIGN,
		nyan_op::ADD_ASSIGN,
		nyan_op::UNION_ASSIGN,
		nyan_op::INTERSECT_ASSIGN,
	};

	switch (with_type.get_container_type()) {
	case container_t::SET:
	case container_t::ORDEREDSET:
		return set_ops;

	case container_t::DICT:
		return dict_ops;

	default:
		return no_nyan_ops;
	}
}

} // namespace nyan
