#pragma once 

#include <src/raze/math/PopulationCount.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M>
raze_always_inline i32 count_set_(M x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) return x;
	else return math::popcnt_n_bits<N>(to_mask_<ISA, T>(x));
}

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask>
raze_always_inline i32 count_set_(M x, ControlMask mask) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) return mask && x;
	else return math::popcnt_n_bits<N>(to_mask_<ISA, T>(mask_and_<ISA, T>(x, mask)));
}

__RAZE_VX_NAMESPACE_END
