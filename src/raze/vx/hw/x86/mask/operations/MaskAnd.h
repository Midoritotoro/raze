#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M>
raze_always_inline M mask_and_(M x, M y) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) return x && y;
	else if constexpr (intrin_type<M>) return bit_and_<ISA, T>(x, y);
	else if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>) return _kand_mask8(x, y);
	else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>) return _kand_mask16(x, y);
	else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>) return _kand_mask32(x, y);
	else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>) return _kand_mask64(x, y);
	else return x & y;
}

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask>
raze_always_inline M mask_and_(M x, M y, ControlMask mask) noexcept {
	return mask_and_<ISA, T>(mask_and_<ISA, T>(x, y), mask);
}

__RAZE_VX_NAMESPACE_END
