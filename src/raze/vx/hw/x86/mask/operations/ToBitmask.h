#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _To_bitmask {
	template <intrin_type _Vector_>
	raze_nodiscard raze_static_operator raze_always_inline auto operator()(_Vector_ __x) raze_const_operator noexcept {
		if constexpr (!__has_avx512bw_support_v<_ISA_> && sizeof(_Type_) == 2) return _To_mask<_ISA_, i8>()(__x);
		else return _To_mask<_ISA_, _Type_>()(__x);
	}
};

__RAZE_VX_NAMESPACE_END
