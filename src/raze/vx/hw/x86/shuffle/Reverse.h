#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/common/PatternsCheck.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __shuffle(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept;

template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ __reverse(_Simd_ __x) noexcept {
	return __shuffle(__x, make_reversed_pattern<_Simd_>{});
}

__RAZE_VX_NAMESPACE_END
