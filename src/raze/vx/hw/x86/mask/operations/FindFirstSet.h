#pragma once 

#include <src/raze/math/CountTrailingZeros.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, bool Unsafe, raw_mask_type M>
raze_always_inline i32 find_first_set_(M x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) return !x;
	else return math::ctz_n_bits<ISA, N, Unsafe>(to_mask_<ISA, T>(x));
}

template <arch::ISA ISA, u32 N, arithmetic_type T, bool Unsafe, raw_mask_type M, raw_mask_type ControlMask>
raze_always_inline i32 find_first_set_(M x, ControlMask mask) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) return !mask || !x;
	else return math::ctz_n_bits<ISA, N, Unsafe>(to_mask_<ISA, T>(mask_and_<ISA, T>(x, mask)));
}

__RAZE_VX_NAMESPACE_END
