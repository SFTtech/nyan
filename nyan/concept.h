// Copyright 2025-2025 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once

#include <concepts>


namespace nyan {

class Value;
class Object;

/**
 * Type that is a nyan value.
 */
template <typename T>
concept ValueLike = std::derived_from<T, Value>;

/**
 * Type that is either a nyan value or object.
 * Object is not a value (ObjectValue is), but want to allow an
 * overloaded conversion for direct object access.
 */
template <typename T>
concept ValueOrObjectLike = std::is_same_v<T, Object> or ValueLike<T>;

} // namespace nyan
