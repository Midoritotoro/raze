#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M1, raw_mask_type M2>
raze_always_inline M1 mask_or_(M1 x, M2 y) noexcept {
	if constexpr (intrin_type<M1> && intrin_type<M2>) return bit_or_<ISA, T>(x, as<M1>(y));
	else if constexpr (std::is_same_v<std::remove_cvref_t<M1>, bool> && std::is_same_v<std::remove_cvref_t<M2>, bool>) return x || y;
#if defined(raze_cpp_msvc_only)
	else if constexpr (sizeof(M1) == 1 && has_avx512dq<ISA>) return _kor_mask8(x, y);
	else if constexpr (sizeof(M1) == 2 && has_avx512f<ISA>) return _kor_mask16(x, y);
	else if constexpr (sizeof(M1) == 4 && has_avx512bw<ISA>) return _kor_mask32(x, y);
	else if constexpr (sizeof(M1) == 8 && has_avx512bw<ISA>) return _kor_mask64(x, y);
#endif
	else return static_cast<M1>(x | y);
}

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M1, raw_mask_type M2, raw_mask_type ControlMask>
raze_always_inline M1 mask_or_(M1 x, M2 y, ControlMask mask) noexcept {
	return mask_and_<ISA, T>(mask_or_<ISA, T>(x, y), mask);
}

__RAZE_VX_NAMESPACE_END
