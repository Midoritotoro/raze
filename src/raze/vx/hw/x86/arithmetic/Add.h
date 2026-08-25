#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V add_(V x, V y) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_add_epi64(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_add_epi32(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_add_epi16(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_add_epi8(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_add_ps(as<__m128>(x), as<__m128>(y)));
		else if constexpr (pd<T>) return as<V>(_mm_add_pd(as<__m128d>(x), as<__m128d>(y)));
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (ps<T>) return as<V>(_mm256_add_ps(as<__m256>(x), as<__m256>(y)));
		else if constexpr (pd<T>) return as<V>(_mm256_add_pd(as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (has_avx2<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_add_epi64(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_add_epi32(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_add_epi16(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_add_epi8(as<__m256i>(x), as<__m256i>(y)));
		}
		else {
			const auto low = add_<arch::ISA::SSE42, T>(as<__m128i>(x), as<__m128i>(y));
			const auto high = add_<arch::ISA::SSE42, T>(
				_mm256_extractf128_si256(as<__m256i>(x), 1), _mm256_extractf128_si256(as<__m256i>(y), 1));

			return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (pd<T>) return as<V>(_mm512_add_pd(as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_add_ps(as<__m512>(x), as<__m512>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_add_epi32(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_add_epi64(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_add_epi8(as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_add_epi16(as<__m512i>(x), as<__m512i>(y)));
		}
		else {
			const auto low = add_<arch::ISA::AVX2, T>(as<__m256i>(x), as<__m256i>(y));
			const auto high = add_<arch::ISA::AVX2, T>(
				_mm512_extracti64x4_epi64(as<__m512i>(x), 1), _mm512_extracti64x4_epi64(as<__m512i>(y), 1));

			return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
		}
	}
	else {
		return x + y;
	}
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V add_(V x, V y, M mask) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_maskz_add_epi64(mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_maskz_add_epi32(mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (ps<T>) return as<V>(_mm_maskz_add_ps(mask, as<__m128>(x), as<__m128>(y)));
			else if constexpr (pd<T>) return as<V>(_mm_maskz_add_pd(mask, as<__m128d>(x), as<__m128d>(y)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_maskz_add_epi16(mask, as<__m128i>(x), as<__m128i>(y)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_maskz_add_epi8(mask, as<__m128i>(x), as<__m128i>(y)));
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_maskz_add_epi64(mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_maskz_add_epi32(mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (ps<T>) return as<V>(_mm256_maskz_add_ps(mask, as<__m256>(x), as<__m256>(y)));
			else if constexpr (pd<T>) return as<V>(_mm256_maskz_add_pd(mask, as<__m256d>(x), as<__m256d>(y)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_maskz_add_epi16(mask, as<__m256i>(x), as<__m256i>(y)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_maskz_add_epi8(mask, as<__m256i>(x), as<__m256i>(y)));
			}
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_maskz_add_epi64(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_add_epi32(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_add_ps(mask, as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_add_pd(mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_maskz_add_epi16(mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_maskz_add_epi8(mask, as<__m512i>(x), as<__m512i>(y)));
		}

	}

	return select_<ISA, T>(add_<ISA, T>(x, y), mask);
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V add_(V x, V y, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_mask_add_epi64(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_mask_add_epi32(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
			else if constexpr (ps<T>) return as<V>(_mm_mask_add_ps(as<__m128>(src), mask, as<__m128>(x), as<__m128>(y)));
			else if constexpr (pd<T>) return as<V>(_mm_mask_add_pd(as<__m128d>(src), mask, as<__m128d>(x), as<__m128d>(y)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_mask_add_epi16(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_mask_add_epi8(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_mask_add_epi64(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_mask_add_epi32(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (ps<T>) return as<V>(_mm256_mask_add_ps(as<__m256>(src), mask, as<__m256>(x), as<__m256>(y)));
			else if constexpr (pd<T>) return as<V>(_mm256_mask_add_pd(as<__m256d>(src), mask, as<__m256d>(x), as<__m256d>(y)));
			else if constexpr (has_avx512bw<ISA>) {
				if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_mask_add_epi16(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
				else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_mask_add_epi8(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
			}
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_mask_add_epi64(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_add_epi32(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_add_ps(as<__m512>(src), mask, as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_add_pd(as<__m512d>(src), mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_mask_add_epi16(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_mask_add_epi8(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		}
	}

	return select_<ISA, T>(add_<ISA, T>(x, y), src, mask);
}

__RAZE_VX_NAMESPACE_END
