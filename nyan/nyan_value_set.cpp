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


NyanContainer::iterator NyanSet::begin() {
	// does semi-magic:
	// We create a heap-allocated NyanSetIterator.
	// It is designed to be callable by a generic interface
	// that all NyanContainer support,
	// but specifically relays calls in the way NyanSet needs it.
	//
	// iterator::elem_type = the single element type of the iteration.
	// NyanSet             = the target set class,
	//                       which is non-const in this begin()
	//                       implementation, but not in the begin() below.
	// this, true          = use this set as target, use the beginning.
	auto real_iterator = std::make_unique<
		NyanSetIterator<iterator::elem_type, NyanSet>>(this, true);

	return iterator{std::move(real_iterator)};
}


NyanContainer::iterator NyanSet::end() {
	// see explanation in the begin() above
	auto real_iterator = std::make_unique<
		NyanSetIterator<iterator::elem_type, NyanSet>>(this, false);

	return iterator{std::move(real_iterator)};
}


NyanContainer::const_iterator NyanSet::begin() const {
	// see explanation in the begin() above
	auto real_iterator = std::make_unique<
		NyanSetIterator<const_iterator::elem_type,
		                const NyanSet>>(this, true);

	return const_iterator{std::move(real_iterator)};
}


NyanContainer::const_iterator NyanSet::end() const {
	// see explanation in the begin() above
	auto real_iterator = std::make_unique<
		NyanSetIterator<const_iterator::elem_type,
		                const NyanSet>>(this, false);

	return const_iterator{std::move(real_iterator)};
}


NyanSet::value_storage::iterator NyanSet::values_begin() {
	return this->values.begin();
}


NyanSet::value_storage::iterator NyanSet::values_end() {
	return this->values.end();
}


NyanSet::value_storage::const_iterator NyanSet::values_begin() const {
	return this->values.begin();
}


NyanSet::value_storage::const_iterator NyanSet::values_end() const {
	return this->values.end();
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
