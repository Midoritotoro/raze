#pragma once

#include <src/raze/vx/Concepts.h>
#include <limits.h>

__RAZE_MATH_NAMESPACE_BEGIN

template <vx::arithmetic_type R, vx::arithmetic_type T>
raze_always_inline R saturating_cast(T x) noexcept {
	constexpr int digits_r = std::numeric_limits<R>::digits;
	constexpr int digits_t = std::numeric_limits<T>::digits;

	constexpr R max_out = std::numeric_limits<R>::max();
	constexpr R min_out = std::integral<R> ? std::numeric_limits<R>::min() : std::numeric_limits<R>::lowest();

	if constexpr (std::floating_point<T> && std::floating_point<R>) {
		if constexpr (sizeof(R) < sizeof(T)) {
			if (x != x) return std::numeric_limits<R>::quiet_NaN();
			if (x > static_cast<T>(max_out)) return max_out;
			if (x < static_cast<T>(min_out)) return min_out;
		}
	}
	else if constexpr (std::floating_point<T>) {
		if (x != x) return 0;
		if (x >= static_cast<T>(max_out)) return max_out;
		if (x <= static_cast<T>(min_out)) return min_out;
		return static_cast<R>(x);
	}
	else if constexpr (std::integral<R> && std::integral<T> && std::signed_integral<R> == std::signed_integral<T>) {
		if constexpr (digits_r < digits_t) {
			if (x < static_cast<T>(min_out)) return min_out;
			else if (x > static_cast<T>(max_out)) return max_out;
		}
	}
	else if constexpr (std::signed_integral<T> && std::unsigned_integral<R>) {
		if (x < 0) return 0;
		else if (std::make_unsigned_t<T>(x) > max_out) return max_out;
	}
	else if constexpr (std::unsigned_integral<T> && std::signed_integral<R>) {
		if (x > std::make_unsigned_t<R>(max_out)) return max_out;
	}

	return static_cast<R>(x);
}

__RAZE_MATH_NAMESPACE_END
