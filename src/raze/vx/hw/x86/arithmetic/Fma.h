#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Mul.h>
#include <src/raze/vx/hw/x86/arithmetic/Add.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V fma_(V x, V y, V z) noexcept {
	if constexpr (sizeof(V) == 16 && has_fma3<ISA>) {
		if constexpr (pd<T>) return as<V>(_mm_fmadd_pd(as<__m128d>(x), as<__m128d>(y), as<__m128d>(z)));
		else if constexpr (ps<T>) return as<V>(_mm_fmadd_ps(as<__m128>(x), as<__m128>(y), as<__m128>(z)));
	}
	else if constexpr (sizeof(V) == 32 && has_fma3<ISA>) {
		if constexpr (pd<T>) return as<V>(_mm256_fmadd_pd(as<__m256d>(x), as<__m256d>(y), as<__m256d>(z)));
		else if constexpr (ps<T>) return as<V>(_mm256_fmadd_ps(as<__m256>(x), as<__m256>(y), as<__m256>(z)));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (pd<T>) return as<V>(_mm512_fmadd_pd(as<__m512d>(x), as<__m512d>(y), as<__m512d>(z)));
		else if constexpr (ps<T>) return as<V>(_mm512_fmadd_ps(as<__m512>(x), as<__m512>(y), as<__m512>(z)));
	}

	return add_<ISA, T>(mul_<ISA, T>(x, y), z);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V operator()(V x, V y, V z, M mask) noexcept {
	return select_<ISA, T>(fma_<ISA, T>(x, y, z), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V operator()(V x, V y, V z, M mask, V src) noexcept {
	return select_<ISA, T>(fma_<ISA, T>(x, y, z), src, mask);
}

__RAZE_VX_NAMESPACE_END
