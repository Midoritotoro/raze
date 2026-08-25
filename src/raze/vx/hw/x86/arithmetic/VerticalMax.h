#pragma once 

#include <src/raze/vx/hw/x86/compare/Greater.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V vertical_max_(V x, V y) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi16<T>) return as<V>(_mm_max_epi16(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epu8<T>) return as<V>(_mm_max_epu8(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_max_ps(as<__m128>(x), as<__m128>(y)));
		else if constexpr (pd<T>) return as<V>(_mm_max_pd(as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (has_sse41<ISA>) {
			if constexpr (epi32<T>) return as<V>(_mm_max_epi32(as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu32<T>) return as<V>(_mm_max_epu32(as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm_max_epu16(as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm_max_epi8(as<__m128i>(x), as<__m128i>(y)));
		}
		else if constexpr (has_avx512vl<ISA>) {
			if constexpr (epi64<T>) return as<V>(_mm_max_epi64(as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu64<T>) return as<V>(_mm_max_epu64(as<__m128i>(x), as<__m128i>(y)));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (ps<T>) return as<V>(_mm256_max_ps(as<__m256>(x), as<__m256>(y)));
		else if constexpr (ps<T>) return as<V>(_mm256_max_pd(as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (has_avx2<ISA>) {
			if constexpr (epi32<T>) return as<V>(_mm256_max_epi32(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu32<T>) return as<V>(_mm256_max_epu32(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi16<T>) return as<V>(_mm256_max_epi16(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm256_max_epu16(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm256_max_epi8(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm256_max_epu8(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (ps<T>) return as<V>(_mm256_max_ps(as<__m256>(x), as<__m256>(y)));
			else if constexpr (ps<T>) return as<V>(_mm256_max_pd(as<__m256d>(x), as<__m256d>(y)));
		}
		else if constexpr (has_avx512vl<ISA>) {
			if constexpr (epi64<T>) return as<V>(_mm256_max_epi64(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu64<T>) return as<V>(_mm256_max_epu64(as<__m256i>(x), as<__m256i>(y)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_max_epi64(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm512_max_epu64(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm512_max_epi32(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm512_max_epu32(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_max_ps(as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_max_pd(as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi8<T>) return as<V>(_mm512_max_epi8(as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm512_max_epu8(as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epi16<T>) return as<V>(_mm512_max_epi16(as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm512_max_epu16(as<__m512i>(x), as<__m512i>(y)));
		}
	}

	return select_<ISA, T>(x, y, greater_<ISA, T>(x, y));
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V vertical_max_(V x, V y, M mask) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm_maskz_max_epi64(mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm_maskz_max_epu64(mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm_maskz_max_epi32(mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm_maskz_max_epu32(mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_maskz_max_ps(mask, as<__m128>(x), as<__m128>(y)));
		else if constexpr (pd<T>) return as<V>(_mm_maskz_max_pd(mask, as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm_maskz_max_epi16(mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm_maskz_max_epu16(mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm_maskz_max_epi8(mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm_maskz_max_epu8(mask, as<__m128i>(x), as<__m128i>(y)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm256_maskz_max_epi64(mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm256_maskz_max_epu64(mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm256_maskz_max_epi32(mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm256_maskz_max_epu32(mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm256_maskz_max_ps(mask, as<__m256>(x), as<__m256>(y)));
		else if constexpr (pd<T>) return as<V>(_mm256_maskz_max_pd(mask, as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm256_maskz_max_epi16(mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm256_maskz_max_epu16(mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm256_maskz_max_epi8(mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm256_maskz_max_epu8(mask, as<__m256i>(x), as<__m256i>(y)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_maskz_max_epi64(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm512_maskz_max_epu64(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm512_maskz_max_epi32(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm512_maskz_max_epu32(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_max_ps(mask, as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_max_pd(mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_maskz_max_epi16(mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm512_maskz_max_epu16(mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm512_maskz_max_epi8(mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm512_maskz_max_epu8(mask, as<__m512i>(x), as<__m512i>(y)));
		}
	}

	return select_<ISA, T>(vertical_max_<ISA, T>(x, y), mask);
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V vertical_max_(V x, V y, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm_mask_max_epi64(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm_mask_max_epu64(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm_mask_max_epi32(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm_mask_max_epu32(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_mask_max_ps(as<__m128>(src), mask, as<__m128>(x), as<__m128>(y)));
		else if constexpr (pd<T>) return as<V>(_mm_mask_max_pd(as<__m128d>(src), mask, as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm_mask_max_epi16(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm_mask_max_epu16(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm_mask_max_epi8(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm_mask_max_epu8(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm256_mask_max_epi64(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm256_mask_max_epu64(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm256_mask_max_epi32(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm256_mask_max_epu32(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm256_mask_max_ps(as<__m256>(src), mask, as<__m256>(x), as<__m256>(y)));
		else if constexpr (pd<T>) return as<V>(_mm256_mask_max_pd(as<__m256d>(src), mask, as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm256_mask_max_epi16(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm256_mask_max_epu16(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm256_mask_max_epi8(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm256_mask_max_epu8(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_mask_max_epi64(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epu64<T>) return as<V>(_mm512_mask_max_epu64(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epi32<T>) return as<V>(_mm512_mask_max_epi32(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epu32<T>) return as<V>(_mm512_mask_max_epu32(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_max_ps(as<__m512>(src), mask, as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_max_pd(as<__m512d>(src), mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_mask_max_epi16(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epu16<T>) return as<V>(_mm512_mask_max_epu16(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epi8<T>) return as<V>(_mm512_mask_max_epi8(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epu8<T>) return as<V>(_mm512_mask_max_epu8(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		}
	}

	return select_<ISA, T>(vertical_max_<ISA, T>(x, y), src, mask);
}

__RAZE_VX_NAMESPACE_END