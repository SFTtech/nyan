// Copyright 2016-2016 the nyan authors, LGPLv3+. See copying.md for legal info.
#ifndef NYAN_NYAN_CONTAINER_H_
#define NYAN_NYAN_CONTAINER_H_

#include <iterator>

#include "nyan_value.h"


namespace nyan {

class NyanValueContainer;


/**
 * NyanValue that can store other NyanValues.
 */
class NyanContainer : public NyanValue {
public:

	/**
	 * Add the given value to this container.
	 * @returns if the value was added successfully,
	 * false if it was already in there.
	 */
	virtual bool add(NyanValueContainer &&value) = 0;

	/**
	 * Test if this value is in the container.
	 */
	virtual bool contains(NyanValue *value) = 0;

	/**
	 * Remove the given value from the container if it is in there.
	 * @returns if if was removed successfully.
	 */
	virtual bool remove(NyanValue *value) = 0;
};

} // namespace nyan

#endif
