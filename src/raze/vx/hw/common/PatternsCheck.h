#pragma once 

#include <src/raze/vx/hw/common/ShufflePattern.h>
#include <array>

__RAZE_VX_NAMESPACE_BEGIN

inline constexpr sizetype __shuffle_zero = std::numeric_limits<sizetype>::max();

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
consteval bool __is_splat(_Pattern_ __p) noexcept {
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

	bool __found_shift = false;
	sizetype __shift = 0;

	for (sizetype __i = 0; __i < __n; ++__i) {
		const auto __v = __p[__i];

		if (__v == __shuffle_zero)
			continue;

		if (!__found_shift) {
			__shift = __v - __i;
			__found_shift = true;
		}

		if (__v != __i + __shift)
			return false;

		if (__v >= __n)
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_slide_right(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	bool __found_shift = false;
	sizetype __shift = 0;

	for (sizetype __i = 0; __i < __n; ++__i) {
		const auto __v = __p[__i];

		if (__v == __shuffle_zero || __v < 0)
			continue;

		if (!__found_shift) {
			__shift = __i - __v;
			__found_shift = true;
		}
		else {
			if (__i - __v != __shift)
				return false;
		}

		if (__v != __i - __shift)
			return false;

		if (__v >= __n)
			return false;
	}

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
consteval bool __is_zip(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (sizetype __i = 0; __i < __h; ++__i) {
		if (__p[2 * __i] != __i)
			return false;

		if (__p[2 * __i + 1] != (__h + __i))
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval bool __is_unzip(_Pattern_ __p) noexcept {
	constexpr auto __n = _Pattern_::size();

	if ((__n & 1) != 0)
		return false;

	constexpr auto __h = __n / 2;

	for (sizetype __i = 0; __i < __h; ++__i) {
		if (__p[__i] != (2 * __i))
			return false;

		if (__p[__i + __h] != (2 * __i + 1))
			return false;
	}

	return true;
}

template <class _Pattern_>
consteval auto __get_rotate_left_shift(_Pattern_ __p) noexcept
	requires (__is_rotate_left(_Pattern_{}))
{
	return __p[0];
}

template <class _Pattern_>
consteval auto __get_rotate_right_shift(_Pattern_ __p) noexcept
	requires (__is_rotate_right(_Pattern_{}))
{
	constexpr auto __n = _Pattern_::size();
	return (__n - __p[0]) % __n;
}

template <class _Pattern_>
consteval auto __get_slide_left_shift(_Pattern_ __p) noexcept
	requires (__is_slide_left(_Pattern_{}))
{
	for (sizetype __i = 0; __i < _Pattern_::size(); ++__i)
		if (__p[__i] != __shuffle_zero)
			return __p[__i] - __i;

	return _Pattern_::size();
}

template <class _Pattern_>
consteval auto __get_slide_right_shift(_Pattern_ __p) noexcept
	requires (__is_slide_right(_Pattern_{}))
{
	for (sizetype __i = 0; __i < _Pattern_::size(); ++__i)
		if (__p[__i] != __shuffle_zero)
			return __i - __p[__i];

	return _Pattern_::size();
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

template <simd_type _Simd_, auto _Fn_, sizetype _Offset_, sizetype ... _Indices_>
consteval auto __make_shuffle_pattern_with_offset_impl(std::integer_sequence<sizetype, _Indices_...>) noexcept {
	return _Shuffle_pattern<_Simd_, _Fn_(_Indices_ + _Offset_)...>{};
}

template <simd_type _Simd_, auto _Fn_, sizetype _Offset_>
using make_shuffle_pattern_with_offset = decltype(__make_shuffle_pattern_with_offset_impl<_Simd_, _Fn_, _Offset_>(
	std::make_integer_sequence<sizetype, _Simd_::size()>{}));

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
using make_splat_pattern = make_shuffle_pattern<_Simd_, [] (sizetype) { return _Index_; }>;

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

template <simd_type _Simd_>
using make_zip_pattern = make_shuffle_pattern<_Simd_,
	[] (sizetype __i) {
		if ((__i & 1) == 0)
			return __i / 2;

		return _Simd_::size() / 2 + __i / 2;
	}>;

template <simd_type _Simd_>
using make_unzip_pattern = make_shuffle_pattern<_Simd_,
	[] (sizetype __i) {
		if (__i < _Simd_::size() / 2)
			return 2 * __i;

		return 2 * (__i - _Simd_::size() / 2) + 1;
	}>;

__RAZE_VX_NAMESPACE_END
