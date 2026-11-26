#pragma once

#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <src/raze/math/IntegralTypesConversions.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M> 
raze_always_inline M slide_right_(M x, i32 shift) noexcept {
	static constexpr auto size = static_cast<i32>(sizeof(V) / sizeof(T));
	
	if constexpr (intrin_type<M>)
		return slide_right_<ISA, T>(x, shift);
	else if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>)
		return (shift == 0) ? x : false;
	else {
		if (shift >= size)
			return static_cast<M>(0);

		if constexpr (static_cast<sizetype>(size) < sizeof(M) * 8) {
			constexpr auto all_mask = static_cast<M>((M(1) << size) - 1);
			return static_cast<M>((x << shift) & all_mask);
		}
		else {
			return static_cast<M>(x << shift);
		}
	}
}

template <arch::ISA ISA, intrin_type V, arithmetic_type T, raw_mask_type M, i32 Shift>
raze_always_inline M slide_right_(M x, std::integral_constant<i32, Shift> shift) noexcept {
	static constexpr auto size = static_cast<i32>(sizeof(V) / sizeof(T));

	if constexpr (intrin_type<M>) {
		return slide_right_<ISA, T>(x, shift);
	}
	else if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return (Shift == 0) ? x : false;
	}
	else if constexpr (Shift >= size) {
		return static_cast<M>(0);
	}
	else {
		[[maybe_unused]] constexpr auto all_mask = ((sizeof(M) * 8) == static_cast<sizetype>(size))
			? math::max_limit<M>() : static_cast<M>((M(1) << size) - 1);

		if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>) {
			if constexpr (size < 8)
				return mask_and_<ISA, T>(_kshiftli_mask8(x, Shift), all_mask);
			else
				return _kshiftli_mask8(x, Shift);
		}
		else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>) {
			if constexpr (size < 16)
				return mask_and_<ISA, T>(_kshiftli_mask16(x, Shift), all_mask);
			else
				return _kshiftli_mask16(x, Shift);
		}
		else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>) {
			if constexpr (size < 32)
				return mask_and_<ISA, T>(_kshiftli_mask32(x, Shift), all_mask);
			else
				return _kshiftli_mask32(x, Shift);
		}
		else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>) {
			return _kshiftli_mask64(x, Shift);
		}
		else if constexpr (static_cast<sizetype>(size) < sizeof(M) * 8) {
			return static_cast<M>((x << Shift) & all_mask);
		}
		else {
			return static_cast<M>(x << Shift);
		}
	}
}

__RAZE_VX_NAMESPACE_END
