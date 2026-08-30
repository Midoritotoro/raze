#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M>
raze_always_inline bool any_of_(M x) noexcept {
	return !none_of_<ISA, T>(x);
}

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask>
raze_always_inline bool any_of_(M x, ControlMask mask) noexcept
	requires((intrin_type<M>&& intrin_type<ControlMask>) || (std::unsigned_integral<M> && std::unsigned_integral<ControlMask>))
{
	return !none_of_<ISA, T>(x, mask);
}

__RAZE_VX_NAMESPACE_END
