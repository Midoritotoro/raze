#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V left_shift_(V x, u32 shift) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_sll_epi64(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_sll_epi32(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_sll_epi16(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_sll_epi16(
			_mm_and_si128(as<__m128i>(x), _mm_set1_epi8(0xFFu >> shift)), _mm_cvtsi32_si128(shift)));
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx2<ISA>) {
			if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_sll_epi64(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_sll_epi32(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_sll_epi16(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_sll_epi16(
				_mm256_and_si256(as<__m256i>(x), _mm256_set1_epi8(0xFFu >> shift)), _mm_cvtsi32_si128(shift)));
		}
		else {
			const auto low = left_shift_<arch::ISA::SSE42, T>(as<__m128i>(x), shift);
			const auto high = left_shift_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1), shift);

			return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_sll_epi64(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_sll_epi32(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_sll_epi16(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_sll_epi16(
				_mm512_and_si512(as<__m512i>(x), _mm512_set1_epi8(0xFFull >> shift)), _mm_cvtsi32_si128(shift)));
		}
		else {
			const auto low = as<__m256i>(x);
			const auto high = _mm512_extracti64x4_epi64(as<__m512i>(x), 1);

			const auto low_shifted = left_shift_<arch::ISA::AVX2, T>(low, shift);
			const auto high_shifted = left_shift_<arch::ISA::AVX2, T>(high, shift);

			return as<V>(_mm512_inserti64x4(as<__m512i>(low_shifted), high_shifted, 1));
		}
	}
	else {
		return (shift < sizeof(V) * 8) ? (x << shift) : 0;
	}
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V left_shift_(V x, u32 shift, M mask) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_maskz_sll_epi64(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_maskz_sll_epi32(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_maskz_sll_epi16(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_maskz_sll_epi64(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_maskz_sll_epi32(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_maskz_sll_epi16(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_maskz_sll_epi64(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_sll_epi32(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_maskz_sll_epi16(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		}
	}

	return select_<ISA, T>(_Left_shift()(x, shift), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V left_shift_(V x, u32 shift, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_mask_sll_epi64(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_mask_sll_epi32(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_mask_sll_epi16(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_mask_sll_epi64(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_mask_sll_epi32(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_mask_sll_epi16(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_mask_sll_epi64(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_sll_epi32(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_mask_sll_epi16(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		}
	}

	return select_<ISA, T>(left_shift_<ISA, T>(x, shift), src, mask);
}

__RAZE_VX_NAMESPACE_END
