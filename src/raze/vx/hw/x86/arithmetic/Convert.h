#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V abs_(V x) noexcept {
	if constexpr (arithmetic_type<V>) return static_cast<T>(x);
	else return convert_fallback_
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V abs_(V x, M mask) noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return select_<ISA, T>(x, mask);
	
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return _mm_maskz_abs_epi64(mask, x);
		else if constexpr (epi32<T>) return _mm_maskz_abs_epi32(mask, x);
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return _mm_maskz_abs_epi16(mask, x);
			else if constexpr (epi8<T>) return _mm_maskz_abs_epi8(mask, x);
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return _mm256_maskz_abs_epi64(mask, x);
		else if constexpr (epi32<T>) return _mm256_maskz_abs_epi32(mask, x);
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return _mm256_maskz_abs_epi16(mask,x);
			else if constexpr (epi8<T>) return _mm256_maskz_abs_epi8(mask, x);
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return _mm512_maskz_abs_epi64(mask, x);
		else if constexpr (epi32<T>) return _mm512_maskz_abs_epi32(mask, x);
		else if constexpr (ps<T>) return _mm512_mask_abs_ps(_mm512_setzero_ps(), mask, x);
		else if constexpr (pd<T>) return _mm512_mask_abs_pd(_mm512_setzero_pd(), mask, x);
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return _mm512_maskz_abs_epi16(mask, x);
			else if constexpr (epi8<T>) return _mm512_maskz_abs_epi8(mask, x);
		}
	}
	
	return select_<ISA, T>(abs_<ISA, T>(x), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V abs_(V x, M mask, V src) noexcept {
	if constexpr (std::is_unsigned_v<T>)
		return select_<ISA, T>(x, src, mask);

	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return _mm_mask_abs_epi64(src, mask, x);
		else if constexpr (epi32<T>) return _mm_mask_abs_epi32(src, mask, x);
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return _mm_mask_abs_epi16(src, mask, x);
			else if constexpr (epi8<T>) return _mm_mask_abs_epi8(src, mask, x);
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return _mm256_mask_abs_epi64(src, mask, x);
		else if constexpr (epi32<T>) return _mm256_mask_abs_epi32(src, mask, x);
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return _mm256_mask_abs_epi16(src, mask, x);
			else if constexpr (epi8<T>) return _mm256_mask_abs_epi8(src, mask, x);
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return _mm512_mask_abs_epi64(src, mask, x);
		else if constexpr (epi32<T>) return _mm512_mask_abs_epi32(src, mask, x);
		else if constexpr (ps<T>) return _mm512_mask_abs_ps(src, mask, x);
		else if constexpr (pd<T>) return _mm512_mask_abs_pd(src, mask, x);
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return _mm512_mask_abs_epi16(src, mask, x);
			else if constexpr (epi8<T>) return _mm512_mask_abs_epi8(src, mask, x);
		}
	}
	
	return select_<ISA, T>(abs_<ISA, T>(x), src, mask);
}

__RAZE_VX_NAMESPACE_END