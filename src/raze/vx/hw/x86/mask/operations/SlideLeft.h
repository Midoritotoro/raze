#pragma once

#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/math/IntegralTypesConversions.h>


__RAZE_VX_NAMESPACE_BEGIN


template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M>
raze_always_inline M slide_left_(M x, i32 shift) noexcept {
	static constexpr auto size = static_cast<i32>(sizeof(V) / sizeof(T));
	if constexpr (intrin_type<M>)
		return slide_left_<ISA, T>(x, shift);
	else if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>)
		return (shift == 0) ? x : false;
	else
		return (shift >= size) ? static_cast<M>(0) : static_cast<M>(x >> shift);
}

template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M, i32 Shift>
raze_always_inline M slide_left_(M x, std::integral_constant<i32, Shift> shift) noexcept {
	static constexpr auto size = static_cast<i32>(sizeof(V) / sizeof(T));

	if constexpr (intrin_type<M>)
		return slide_left_<ISA, T>(x, shift);
	else if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>)
		return (Shift == 0) ? x : false;
	else if constexpr (Shift >= size)
		return static_cast<M>(0);
	else if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>)
		return _kshiftri_mask8(x, Shift);
	else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>)
		return _kshiftri_mask16(x, Shift);
	else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>)
		return _kshiftri_mask32(x, Shift);
	else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>)
		return _kshiftri_mask64(x, Shift);
	else
		return static_cast<M>(x >> Shift);
}

__RAZE_VX_NAMESPACE_END
