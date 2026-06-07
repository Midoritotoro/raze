#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __rotate_right(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	return __generic_shuffle(__x, __p);
}

__RAZE_VX_NAMESPACE_END
