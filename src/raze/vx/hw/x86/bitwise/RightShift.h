#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V right_shift_(V x, u32 shift) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm_sra_epi64(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
			else {
				auto high_shifted = _mm_setzero_si128();
				auto low_shifted = _mm_setzero_si128();

				if (shift <= 32) {
					const auto shift_vector = _mm_cvtsi32_si128(shift);
					high_shifted = _mm_sra_epi32(as<__m128i>(x), shift_vector);
					low_shifted = _mm_srl_epi64(as<__m128i>(x), shift_vector);
				}
				else {
					const auto shift_vector = _mm_cvtsi32_si128(static_cast<i32>(shift) - 32);
					high_shifted = _mm_srai_epi32(as<__m128i>(x), 31);
					low_shifted = _mm_srli_epi64(_mm_sra_epi32(as<__m128i>(x), shift_vector), 32);
				}

				return as<V>(select_<ISA, T>(high_shifted, low_shifted, _mm_set_epi32(-1, 0, -1, 0)));
			}
		}
		else if constexpr (epu64<T>) return as<V>(_mm_srl_epi64(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm_sra_epi32(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm_srl_epi32(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi16<T>) return as<V>(_mm_sra_epi16(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu16<T>) return as<V>(_mm_srl_epi16(as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu8<T>) return as<V>(_mm_srl_epi16(
			_mm_and_si128(as<__m128i>(x), _mm_set1_epi8(0xFFu << shift)), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi8<T>) {
			const auto even_shifted = _mm_sra_epi16(_mm_slli_epi16(as<__m128i>(x), 8), _mm_cvtsi32_si128(shift + 8));
			const auto odd_shifted = _mm_sra_epi16(as<__m128i>(x), _mm_cvtsi32_si128(shift));

			if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return as<V>(_mm_mask_mov_epi8(odd_shifted, 0x5555, even_shifted));
			else return as<V>(select_<ISA, T>(even_shifted, odd_shifted, _mm_set1_epi32(0x00FF00FF)));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx2<ISA>) {
			if constexpr (epi64<T>) {
				if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_sra_epi64(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
				else {
					auto high_shifted = _mm256_setzero_si256();
					auto low_shifted = _mm256_setzero_si256();

					if (shift <= 32) {
						const auto shift_vector = _mm_cvtsi32_si128(shift);
						high_shifted = _mm256_sra_epi32(as<__m256i>(x), shift_vector);
						low_shifted = _mm256_srl_epi64(as<__m256i>(x), shift_vector);
					}
					else {
						high_shifted = _mm256_srai_epi32(as<__m256i>(x), 31);
						low_shifted = _mm256_srli_epi64(_mm256_sra_epi32(as<__m256i>(x), _mm_cvtsi32_si128(shift - 32)), 32);
					}

					return as<V>(_mm256_blend_epi32(low_shifted, high_shifted, 0xAA));
				}
			}
			else if constexpr (epu64<T>) return as<V>(_mm256_srl_epi64(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi32<T>) return as<V>(_mm256_sra_epi32(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu32<T>) return as<V>(_mm256_srl_epi32(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi16<T>) return as<V>(_mm256_sra_epi16(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm256_srl_epi16(as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi8<T>) {
				const auto even_shifted = _mm256_sra_epi16(_mm256_slli_epi16(as<__m256i>(x), 8), _mm_cvtsi32_si128(shift + 8));
				const auto odd_shifted = _mm256_sra_epi16(as<__m256i>(x), _mm_cvtsi32_si128(shift));

				if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return as<V>(_mm256_mask_mov_epi8(odd_shifted, 0x55555555, even_shifted));
				else return as<V>(select_<ISA, T>(even_shifted, odd_shifted, _mm256_set1_epi32(0x00FF00FF)));
			}
			else if constexpr (epu8<T>) {
				const auto and_mask = _mm256_and_si256(as<__m256i>(x), _mm256_set1_epi8(0xFFu << shift));
				return as<V>(_mm256_srl_epi16(and_mask, _mm_cvtsi32_si128(shift)));
			}
		}
		else {
			const auto low = right_shift_<arch::ISA::SSE42, T>(as<__m128i>(x), shift);
			const auto high = right_shift_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1), shift);

			return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_sra_epi64(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm512_srl_epi64(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm512_sra_epi32(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm512_srl_epi32(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_sra_epi16(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm512_srl_epi16(as<__m512i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu8<T>) return as<V>(_mm512_srl_epi16(
				_mm512_and_si512(as<__m512i>(x), _mm512_set1_epi8(0xFFu << shift)), _mm_cvtsi32_si128(shift)));
			else if constexpr (epi8<T>) {
				const auto even_vector = _mm512_slli_epi16(as<__m512i>(x), 8);

				const auto even_shifted = _mm512_sra_epi16(even_vector, _mm_cvtsi32_si128(shift + 8));
				const auto odd_shifted = _mm512_sra_epi16(as<__m512i>(x), _mm_cvtsi32_si128(shift));

				return as<V>(_mm512_mask_mov_epi8(even_shifted, 0xAAAAAAAAAAAAAAAA, odd_shifted));
			}
		}
		else {
			const auto low = as<__m256i>(x);
			const auto high = _mm512_extracti64x4_epi64(as<__m512i>(x), 1);

			const auto low_shifted = right_shift_<arch::ISA::AVX2, T>(low, shift);
			const auto high_shifted = right_shift_<arch::ISA::AVX2, T>(high, shift);

			return as<V>(_mm512_inserti64x4(as<__m512i>(low_shifted), high_shifted, 1));
		}
	}
	else {
		V result = 0;

		if (shift < sizeof(T) * 8) {
			result = x >> shift;
		}
		else {
			if constexpr (std::is_signed_v<T>)
				result = x >> (sizeof(T) * 8 - 1);
		}

		return result;
	}
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V right_shift_(V x, u32 shift, M mask) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm_maskz_sra_epi64(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm_maskz_srl_epi64(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm_maskz_sra_epi32(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm_maskz_srl_epi32(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm_maskz_sra_epi16(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm_maskz_srl_epi16(mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm256_maskz_sra_epi64(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm256_maskz_srl_epi64(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm256_maskz_sra_epi32(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm256_maskz_srl_epi32(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm256_maskz_sra_epi16(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm256_maskz_srl_epi16(mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_maskz_sra_epi64(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm512_maskz_srl_epi64(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm512_maskz_sra_epi32(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm512_maskz_srl_epi32(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_maskz_sra_epi16(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm512_maskz_srl_epi16(mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		}
	}

	return select_<ISA, T>(right_shift_<ISA, T>(x, shift), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V right_shift_(V x, u32 shift, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm_mask_sra_epi64(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm_mask_srl_epi64(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm_mask_sra_epi32(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm_mask_srl_epi32(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm_mask_sra_epi16(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm_mask_srl_epi16(as<__m128i>(src), mask, as<__m128i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T>) return as<V>(_mm256_mask_sra_epi64(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm256_mask_srl_epi64(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm256_mask_sra_epi32(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm256_mask_srl_epi32(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm256_mask_sra_epi16(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm256_mask_srl_epi16(as<__m256i>(src), mask, as<__m256i>(x), _mm_cvtsi32_si128(shift)));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T>) return as<V>(_mm512_mask_sra_epi64(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu64<T>) return as<V>(_mm512_mask_srl_epi64(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epi32<T>) return as<V>(_mm512_mask_sra_epi32(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (epu32<T>) return as<V>(_mm512_mask_srl_epi32(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T>) return as<V>(_mm512_mask_sra_epi16(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
			else if constexpr (epu16<T>) return as<V>(_mm512_mask_srl_epi16(as<__m512i>(src), mask, as<__m512i>(x), _mm_cvtsi32_si128(shift)));
		}
	}

	return select_<ISA, T>(right_shift_<ISA, T>(x, shift), src, mask);
}

__RAZE_VX_NAMESPACE_END
