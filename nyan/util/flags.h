// Copyright 2021-2021 the nyan authors, LGPLv3+. See copying.md for legal info.
#pragma once

#include <bitset>
#include <concepts>
#include <cstddef>
#include <cstdint>


namespace nyan::util {

/**
 * Wrapper for an enum so we can use it as bitset.
 *
 * Example:
 *   enum class lol {
 *      red,
 *      green,
 *      yellow
 *   };
 *
 *   Flags<lol> lolflags;
 *   lolflags.set(lol::red);
 *   lolflags[lol::red] == true;
 */
template <typename T>
requires
std::is_enum_v<T> &&
requires {
	{ T::size };
}
class Flags {
public:
	Flags &set(T e, bool value=true) noexcept {
		this->bits.set(underlying(e), value);
		return *this;
	}

	Flags &reset(T e) noexcept {
		this->set(e, false);
		return *this;
	}

	Flags &reset() noexcept {
		this->bits.reset();
		return *this;
	}

	// TODO: maybe add operator&=,|=,^=,~

	[[nodiscard]] bool all() const noexcept {
		return this->bits.all();
	}

	[[nodiscard]] bool any() const noexcept {
		return this->bits.any();
	}

	[[nodiscard]] bool none() const noexcept {
		return this->bits.none();
	}

	[[nodiscard]] constexpr std::size_t size() const noexcept {
		return this->bits.size();
	}

	[[nodiscard]] std::size_t count() const noexcept {
		return this->bits.count();
	}

	constexpr bool operator[](T e) const {
		return this->bits[underlying(e)];
	}

private:
	using enum_val_t = typename std::make_unsigned_t<typename std::underlying_type_t<T>>;

	/**
	 * convert enum entry to underlying value.
	 */
	static constexpr enum_val_t underlying(T e) {
		return static_cast<enum_val_t>(e);
	}

private:
	/**
	 * Bitset with as many entries as the enum has.
	 */
	std::bitset<underlying(T::size)> bits;
};

} // namespace nyan::util
