#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>

__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA ISA, intrin_or_arithmetic_type V, arithmetic_type T>
raze_always_inline V zero_() noexcept {
	if constexpr (std::is_same_v<V, __m128i>) return _mm_setzero_si128();
	else if constexpr (std::is_same_v<V, __m128d>) return _mm_setzero_pd();
	else if constexpr (std::is_same_v<V, __m128>) return _mm_setzero_ps();
	else if constexpr (std::is_same_v<V, __m256i>) return _mm256_setzero_si256();
	else if constexpr (std::is_same_v<V, __m256d>) return _mm256_setzero_pd();
	else if constexpr (std::is_same_v<V, __m256>) return _mm256_setzero_ps();
	else if constexpr (std::is_same_v<V, __m512i>) return _mm512_setzero_si512();
	else if constexpr (std::is_same_v<V, __m512d>) return _mm512_setzero_pd();
	else if constexpr (std::is_same_v<V, __m512>) return _mm512_setzero_ps();
	else return 0;
}

template <arch::ISA ISA, intrin_or_arithmetic_type V, arithmetic_type T, raw_mask_type Mask>
raze_always_inline V zero_(Mask mask) noexcept {
	return select_<ISA, T>(zero_<ISA, V, T>(), mask);
}

template <arch::ISA ISA, intrin_or_arithmetic_type V, arithmetic_type T, raw_mask_type Mask>
raze_always_inline V zero_(Mask mask, V src) noexcept {
	return select_<ISA, T>(zero_<ISA, V, T>(), src, mask);
}

__RAZE_VX_NAMESPACE_END
