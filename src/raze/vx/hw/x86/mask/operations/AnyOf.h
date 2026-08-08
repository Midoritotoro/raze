#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, raw_mask_type _Tp_>
raze_nodiscard raze_always_inline bool __any_of(_Tp_ __x) noexcept {
	return !__none_of<_ISA_, _Type_>(__x);
}

template <arch::ISA _ISA_, arithmetic_type _Type_, raw_mask_type _Tp_, raw_mask_type _Mask_>
raze_nodiscard raze_always_inline bool __any_of(_Tp_ __x, _Mask_ __mask) noexcept
	requires((intrin_type<_Tp_>&& intrin_type<_Mask_>) || (std::unsigned_integral<_Tp_> && std::unsigned_integral<_Mask_>))
{
	return !__none_of<_ISA_, _Type_>(__x, __mask);
}

__RAZE_VX_NAMESPACE_END
