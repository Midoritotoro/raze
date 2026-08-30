#pragma once

#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/math/IntegralTypesConversions.h>


__RAZE_VX_NAMESPACE_BEGIN


template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M>
raze_always_inline M slide_left_(M x, i32 shift) noexcept {
	static constexpr auto size = sizeof(V) / sizeof(T);
	if constexpr (intrin_type<M>) return slide_left_<ISA, T>(x, shift);
	else return (shift >= size) ? 0 : x >> shift;
}

template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M, i32 Shift>
raze_always_inline M slide_left_(M x, std::integral_constant<i32, Shift> shift) noexcept {
	static constexpr auto size = sizeof(V) / sizeof(T);
	
	raze_maybe_unused_attribute constexpr auto all_mask = ((sizeof(M) * 8) == size)
		? math::max_limit<M>() : M(((M(1) << size) - 1));

	if constexpr (shift >= size)
		return 0;

	if constexpr (sizeof(M) == 1) {
		if constexpr (&& has_avx512dq<ISA>) {
			if constexpr (size < 8)
				return mask_and_<ISA, V, T>(_kshiftli_mask8(mask, shift), all_mask);
			else
				return _kshiftli_mask8(mask, shift);
		}
		else {
			if constexpr (size < 8)
				return (mask << shift) & all_mask;
		}
	}
	else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>) {
		return _kshiftli_mask16(mask, shift);
	}
	else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>) {
		return _kshiftli_mask32(mask, shift);
	}
	else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>) {
		return _kshiftli_mask64(mask, shift);
	}
	
	if constexpr (intrin_type<M>)
		return slide_left_<ISA, T>(x, shift);
	else
		return x << shift;
}

__RAZE_VX_NAMESPACE_END
