#pragma once 

#include <src/raze/vx/hw/common/ShufflePattern.h>
#include <array>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
consteval auto __is_reverse(_Pattern_ __p) noexcept {
	for (auto __i = 0; __i < _Pattern_::size(); ++__i)
		if (__p[__i] != (_Pattern_::size() - __i - 1))
			return false;

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

template <simd_type _Simd_, sizetype ... _Indices_>
constexpr auto __reverse_shuffle_pattern(std::integer_sequence<sizetype, _Indices_...>) noexcept {
	return _Shuffle_pattern<_Simd_, (sizeof...(_Indices_) - 1 - _Indices_)...>{};
}

template <simd_type _Simd_>
using make_reversed_pattern = decltype(__reverse_shuffle_pattern<_Simd_>(std::make_integer_sequence<u64, _Simd_::size()>{}));


__RAZE_VX_NAMESPACE_END
