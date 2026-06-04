#pragma once 

#include <src/raze/vx/hw/common/ShufflePattern.h>
#include <array>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
consteval bool __is_reverse(_Pattern_ __p) noexcept {
	for (auto __i = 0; __i < _Pattern_::size(); ++__i)
		if (__p[__i] != (_Pattern_::size() - __i - 1))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_identity(_Pattern_ __p) noexcept {
	for (auto __i = 0; __i < _Pattern_::size(); ++__i)
		if (__p[__i] != __i)
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_broadcast(_Pattern_ __p) noexcept {
	const auto __v = __p[0];

	for (auto __i = 1; __i < _Pattern_::size(); ++__i)
		if (__p[__i] != __v)
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_rotate_left(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	const auto __shift = __p[0];

	for (auto __i = 0; __i < __n; ++__i)
		if (__p[__i] != ((__i + __shift) % __n))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_rotate_right(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();
	const auto __shift = (__n - __p[0]) % __n;

	for (auto __i = 0; __i < __n; ++__i)
		if (__p[__i] != ((__i + __n - __shift) % __n))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_slide_left(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	const auto __shift = __p[0];

	if (__shift >= __n)
		return false;

	for (auto __i = 0; __i + __shift < __n; ++__i)
		if (__p[__i] != (__i + __shift))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_slide_right(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	const auto __shift = __n - __p[__n - 1] - 1;

	if (__shift >= __n)
		return false;

	for (auto __i = __shift; __i < __n; ++__i)
		if (__p[__i] != (__i - __shift))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_interleave_low(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (auto __i = 0; __i < __h; ++__i) {
		if (__p[2 * __i] != __i)
			return false;

		if (__p[2 * __i + 1] != (__h + __i))
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_interleave_high(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (auto __i = 0; __i < __h; ++__i) {
		if (__p[2 * __i] != (__h + __i))
			return false;

		if (__p[2 * __i + 1] != __i)
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval auto __across_halfs(_Pattern_ __p) noexcept 
	requires (pattern_vector_t<_Pattern_>::is_native())
{
	constexpr auto __size = sizeof(typename _Pattern_::vector_type);

	for (auto __i = 0; __i < _Pattern_::size() / 2; ++__i)
		if (__p[__i] >= (_Pattern_::size() / 2))
			return true;

	for (auto __i = _Pattern_::size() / 2; __i < _Pattern_::size(); ++__i)
		if (__p[__i] < (_Pattern_::size() / 2))
			return true;

	return false;
}

template <class _Pattern_>
consteval auto __across_quads(_Pattern_ __p) noexcept
	requires (pattern_vector_t<_Pattern_>::is_native())
{
	constexpr auto __q = _Pattern_::size() / 4;

	for (auto __i = 0; __i < __q; ++__i)
		if (__p[__i] >= __q)
			return true;

	for (auto __i = __q; __i < 2 * __q; ++__i)
		if (__p[__i] < __q || __p[__i] >= 2 * __q)
			return true;

	for (auto __i = 2 * __q; __i < 3 * __q; ++__i)
		if (__p[__i] < 2 * __q || __p[__i] >= 3 * __q)
			return true;

	for (auto __i = 3 * __q; __i < 4 * __q; ++__i)
		if (__p[__i] < 3 * __q)
			return true;

	return false;
}

template <class _Pattern_>
consteval bool __can_widen_shuffle(_Pattern_ __p) noexcept
	requires((_Pattern_::size() & 1) == 0)
{
	for (auto __i = 0; __i < _Pattern_::size(); __i += 2) {
		const auto __a = __p[__i];
		const auto __b = __p[__i + 1];

		if ((__a & 1) != 0)
			return false;

		if (__b != __a + 1)
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_low_half(_Pattern_ __p) noexcept {
	for (auto __i = 0; __i < __p.size(); ++__i)
		if (__p[__i] >= (__p.size() / 2))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_high_half(_Pattern_ __p) noexcept {
	for (auto __i = 0; __i < __p.size(); ++__i)
		if (__p[__i] < (__p.size() / 2))
			return false;

	return true;
}

template <class _Pattern_>
consteval bool __is_dup_low(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (auto __i = 0; __i < __h; ++__i) {
		if (__p[__i] >= __h)
			return false;

		if (__p[__i + __h] != __p[__i])
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_dup_high(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (auto __i = 0; __i < __h; ++__i) {
		if (__p[__i] < __h || __p[__i] >= __n)
			return false;

		if (__p[__i + __h] != __p[__i])
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_dup_low_identity(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (auto __i = 0; __i < __h; ++__i) {
		if (__p[__i] >= __h || __p[__i] != __i) return false;
		if (__p[__i + __h] != __p[__i]) return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_dup_high_identity(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (auto __i = 0; __i < __h; ++__i) {
		if (__p[__i] < __h || __p[__i] >= __n || __p[__i] != __i) return false;
		if (__p[__i + __h] != __p[__i]) return false;
	}

	return true;
}

template <class _Pattern_>
consteval u8 __to_pshufd_mask(_Pattern_ __p) noexcept {
	return ((__p[0] & 0x03) | ((__p[1] & 0x03) << 2)
		| ((__p[2] & 0x03) << 4) | ((__p[3] & 0x03) << 6));
}

template <class _Pattern_>
consteval u8 __shufpd_to_pshufd_mask(_Pattern_ __p) noexcept {
	return ((2 * (__p[0] & 0x03)) | ((2 * (__p[0] & 0x03) + 1) << 2)
		| ((2 * (__p[1] & 0x03) << 4) | ((2 * (__p[1] & 0x03) + 1) << 6)));
}

template <simd_type _Simd_, auto _Fn_, sizetype ... _Indices_>
consteval auto __make_shuffle_pattern_impl(std::integer_sequence<sizetype, _Indices_...>) noexcept {
	return _Shuffle_pattern<_Simd_, _Fn_(_Indices_)...>{};
}

template <simd_type _Simd_, auto _Fn_>
using make_shuffle_pattern = decltype(__make_shuffle_pattern_impl<_Simd_, _Fn_>(
		std::make_integer_sequence<sizetype, _Simd_::size()>{}));

inline constexpr sizetype __shuffle_zero = std::numeric_limits<sizetype>::max();

template <simd_type _Simd_, sizetype ... _Indices_>
using make_pattern = _Shuffle_pattern<_Simd_, _Indices_...>;

template <simd_type _Simd_>
using make_reversed_pattern = make_shuffle_pattern<_Simd_,
	[] (sizetype __i) { return _Simd_::size() - 1 - __i; }>;

template <simd_type _Simd_>
using make_identity_pattern = make_shuffle_pattern<_Simd_,
	[] (sizetype __i) { return __i; }>;

template <simd_type _Simd_, sizetype _Shift_>
using make_rotate_left_pattern = make_shuffle_pattern<_Simd_, 
	[] (sizetype __i) { return (__i + _Shift_) % _Simd_::size(); }>;

template <simd_type _Simd_, sizetype _Shift_>
using make_rotate_right_pattern = make_shuffle_pattern<_Simd_, 
	[] (sizetype __i) { return (__i + _Simd_::size() - (_Shift_ % _Simd_::size())) % _Simd_::size(); }>;

template <simd_type _Simd_, sizetype _Index_>
using make_broadcast_pattern = make_shuffle_pattern<_Simd_, [] (sizetype) { return _Index_; }>;

template <simd_type _Simd_, sizetype _Shift_>
using make_slide_left_pattern = make_shuffle_pattern<_Simd_,
	[] (sizetype __i) {
		const auto __src = __i + _Shift_;

		if (__src >= _Simd_::size())
			return __shuffle_zero;

		return __src;
	}>;

template <simd_type _Simd_, sizetype _Shift_>
using make_slide_right_pattern = make_shuffle_pattern<_Simd_,
	[] (sizetype __i) {
		if (__i < _Shift_)
			return __shuffle_zero;

		return __i - _Shift_;
	}>;

__RAZE_VX_NAMESPACE_END
