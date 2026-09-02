#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Mul.h>
#include <src/raze/vx/hw/x86/arithmetic/Sub.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V fms_(V x, V y, V z) noexcept {
	if constexpr (sizeof(V) == 16 && has_fma3<ISA>) {
		if constexpr (pd<T>) return _mm_fmsub_pd(x, y, z);
		else if constexpr (ps<T>) return _mm_fmsub_ps(x, y, z);
	}
	else if constexpr (sizeof(V) == 32 && has_fma3<ISA>) {
		if constexpr (pd<T>) return _mm256_fmsub_pd(x, y, z);
		else if constexpr (ps<T>) return _mm256_fmsub_ps(x, y, z);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (pd<T>) return _mm512_fmsub_pd(x, y, z);
		else if constexpr (ps<T>) return _mm512_fmsub_ps(x, y, z);
	}

	return sub_<ISA, T>(mul_<ISA, T>(x, y), z);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V fms_(V x, V y, V z, M mask) noexcept {
	return select_<ISA, T>(fms_<ISA, T>(x, y, z), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V fms_(V x, V y, V z, M mask, V src) noexcept {
	return select_<ISA, T>(fms_<ISA, T>(x, y, z), src, mask);
}

__RAZE_VX_NAMESPACE_END
