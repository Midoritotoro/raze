#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/FirstN.h>
#include <src/raze/vx/hw/x86/mask/operations/BitLeftShift.h>
#include <src/raze/vx/hw/x86/mask/operations/BitRightShift.h>
#include <src/raze/math/CountTrailingZeros.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M>
raze_always_inline bool is_contiguous_(M mask, i32 n, i32 k) noexcept {
	if (k <= n)
		return true;

	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return mask != 0;
	}
	else if constexpr (std::is_integral_v<M>) {
		const auto len = static_cast<u32>(k - n);
		const u64 inv = (~static_cast<u64>(mask)) >> static_cast<u32>(n);

		if constexpr (N == 64) {
			if (inv == 0)
				return true;
			return static_cast<u32>(math::ctz_n_bits<ISA, 64, true>(inv)) >= len;
		}
		else if constexpr (has_bmi2<ISA>) {
			return _bzhi_u64(inv, len) == 0;
		}
		else {
			return (inv & ((1ull << len) - 1ull)) == 0;
		}
	}
	else {
		return is_contiguous_<ISA, N, T>(to_mask_<ISA, T>(mask), n, k);
	}
}

__RAZE_VX_NAMESPACE_END
