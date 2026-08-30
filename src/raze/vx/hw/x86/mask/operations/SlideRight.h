#pragma once

#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/mask/operations/BitAnd.h>
#include <src/raze/math/IntegralTypesConversions.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M> 
raze_always_inline M slide_right_(M x, i32 shift) noexcept {
	static constexpr auto size = sizeof(V) / sizeof(T);
	
	if constexpr (intrin_type<M>)
		return slide_right_<ISA, T>(x, shift);
	else
		return (shift >= size) ? 0 : x >> shift;
}

template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M, i32 Shift>
raze_always_inline M slide_right_(M x, std::integral_constant<i32, Shift> shift) noexcept {
	static constexpr auto size = sizeof(V) / sizeof(T);

	if constexpr (shift >= size)
		return 0;

	if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>)
		return _kshiftri_mask8(x, shift);

	else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>)
		return _kshiftri_mask16(x, shift);

	else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>)
		return _kshiftri_mask32(x, shift);

	else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>)
		return _kshiftri_mask64(x, shift);

	else if constexpr (intrin_type<M>)
		return slide_right_<ISA, T>(x, shift);

	else
		return x >> shift;
}

__RAZE_VX_NAMESPACE_END
