#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <src/raze/vx/hw/x86/mask/operations/FirstN.h>
#include <src/raze/vx/hw/x86/mask/operations/BitLeftShift.h>
#include <src/raze/vx/hw/x86/mask/operations/BitRightShift.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M>
raze_always_inline bool is_contiguous_(M mask, i32 n, i32 k) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return (k <= n) ? true : mask != 0;
	}
	else if constexpr (std::is_integral_v<M>) {
		auto len = k - n;
		
		if constexpr (N == 64) return _tzcnt_u64((~mask) >> n) >= len;
		else if constexpr (has_avx2<ISA>) return _bzhi_u64((~mask) >> n, len) == 0;
		else return (((~mask) >> n) & ((1ull << len) - 1)) == 0; // Compilers usually turn this code into bzhi,
		// but for compatibility with MSVC Runtime Dispatch, this is implemented manually
	}
	else {
		return is_contiguous_<ISA, N, T>(to_mask_<ISA, T>(mask), n, k);
	}
}

__RAZE_VX_NAMESPACE_END
