// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_MEMBER_H_
#define NYAN_NYAN_MEMBER_H_

#include <memory>
#include <string>
#include <unordered_set>

#include "nyan_ops.h"
#include "nyan_value.h"

namespace nyan {

class NyanObject;

/**
 * Stores a member of a NyanObject.
 * Also responsible for validating applied operators.
 */
class NyanMember {
public:
	NyanMember();
	virtual ~NyanMember() = default;

	std::string str();
	virtual NyanValue *get() const = 0;

	nyan_op get_operation() const;

	void cache_save(std::unique_ptr<NyanValue> &&value);
	NyanValue *cache_get() const;
	void cache_reset();

protected:
	nyan_op operation;

	std::unique_ptr<NyanValue> cached_value;
};


/**
 * Member to store a nyan value as owned pointer.
 */
class NyanOwningMember : public NyanMember {
public:
	NyanOwningMember(std::unique_ptr<NyanValue> &&value=nullptr);

	/**
	 * Return the pointer to the contained value.
	 */
	NyanValue *get() const override;

protected:
	std::unique_ptr<NyanValue> value;
};

/**
 * Member to store a value owned elsewhere.
 */
class NyanPtrMember : public NyanMember {
public:
	NyanPtrMember(NyanValue *value=nullptr);

	/**
	 * Return the contained NyanObject pointer.
	 */
	NyanValue *get() const override;

protected:
	NyanValue *value;
};


} // namespace nyan

#endif
