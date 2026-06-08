#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __rotate_right(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	return __generic_shuffle(__x, __p);
}

template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ __rotate_right(const _Simd_& __x, i32 __sh) noexcept {
	alignas(64) typename _Simd_::value_type __arr[_Simd_::size() * 2];

	vx::__store[vx::aligned](__arr, __x);
	vx::__store[vx::aligned](__arr + _Simd_::size(), __x);

	return vx::__load<_Simd_>[vx::aligned](__arr + (_Simd_::size() - __sh));
}

__RAZE_VX_NAMESPACE_END
