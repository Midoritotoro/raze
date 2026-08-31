#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/vx/hw/x86/mask/operations/FirstN.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, raw_mask_type M, arithmetic_type T>
raze_always_inline M mask_broadcast_(bool v) noexcept {
	if constexpr (intrin_type<M>) return v ? broadcast_<ISA, M>(-1) : zero_<ISA, M>();
	else if constexpr (std::is_same_v<M, bool>) return v;
	else return first_n_<ISA, N, M, T>()(N * i32(v));
}

template <arch::ISA ISA, raw_mask_type M>
raze_always_inline M mask_zero_() noexcept {
	if constexpr (intrin_type<M>) return zero_<ISA, M>();
	else return 0;
}

__RAZE_VX_NAMESPACE_END
