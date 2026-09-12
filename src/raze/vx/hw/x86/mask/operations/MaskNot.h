#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, sizetype N, arithmetic_type T, raw_mask_type M>
raze_always_inline M mask_not_(M x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) return !x;
	else if constexpr (intrin_type<M>) return bit_not_<ISA, T>(x);
	else if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>) {
		if constexpr (N < 8) {
			constexpr auto mask = M((M(1) << N) - 1);
			return _kxor_mask8(x, mask);
		}
		else return _knot_mask8(x);
	}
	else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>) return _knot_mask16(x);
	else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>) return _knot_mask32(x);
	else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>) return _knot_mask64(x);
	else {
		if constexpr (N < 8) {
			constexpr auto mask = M((M(1) << N) - 1);
			return x ^ mask;
		}
		else return ~x;
	}
}

template <arch::ISA ISA, sizetype N, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask>
raze_always_inline M mask_not_(M x, ControlMask mask) noexcept {
	return bit_and_<ISA, T>(mask_not_<ISA, N, T>(x), mask);
}

__RAZE_VX_NAMESPACE_END
