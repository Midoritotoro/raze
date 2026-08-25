#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V mul_(V x, V y) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (pd<T>) return as<V>(_mm_mul_pd(as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_mul_ps(as<__m128>(x), as<__m128>(y)));
		else if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx512dq<ISA> && has_avx512vl<ISA>) return as<V>(_mm_mullo_epi64(as<__m128i>(x), as<__m128i>(y)));
			else {
				const auto left_high = _mm_srli_epi64(as<__m128i>(x), 32);
				const auto right_high = _mm_srli_epi64(as<__m128i>(y), 32);

				const auto multiplied1 = _mm_mul_epu32(left_high, as<__m128i>(y));
				const auto multiplied2 = _mm_mul_epu32(right_high, as<__m128i>(x));

				const auto add64 = _mm_add_epi64(multiplied2, multiplied1);

				const auto high = _mm_slli_epi64(add64, 32);
				const auto low = _mm_mul_epu32(as<__m128i>(x), as<__m128i>(y));

				return as<V>(_mm_add_epi64(low, high));
			}
		}
		else if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_sse41<ISA>) return as<V>(_mm_mullo_epi32(as<__m128i>(x), as<__m128i>(y)));
			else {
				const auto shuffled_left = _mm_shuffle_epi32(as<__m128i>(x), 0xF5);
				const auto shuffled_right = _mm_shuffle_epi32(as<__m128i>(y), 0xF5);

				const auto product_02 = _mm_mul_epu32(as<__m128i>(x), as<__m128i>(y));
				const auto product_13 = _mm_mul_epu32(shuffled_left, shuffled_right);

				const auto product_01 = _mm_unpacklo_epi32(product_02, product_13);
				const auto product_23 = _mm_unpackhi_epi32(product_02, product_13);

				return as<V>(_mm_unpacklo_epi64(product_01, product_23));
			}
		}
		else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_mullo_epi16(as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (epi8<T> || epu8<T>) {
			if constexpr (has_avx512bw<ISA> && has_avx512vl<ISA>) {
				const auto converted_x = _mm256_cvtepu8_epi16(as<__m128i>(x));
				const auto converted_y = _mm256_cvtepu8_epi16(as<__m128i>(y));

				const auto multiplied = _mm256_mullo_epi16(converted_x, converted_y);
				return as<V>(_mm256_cvtepi16_epi8(multiplied));
			}
			else if constexpr (has_ssse3<ISA>) {
				const auto low_byte_mask = _mm_set1_epi16(0x00FF);

				auto product_low_words = _mm_mullo_epi16(as<__m128i>(y), as<__m128i>(x));
				product_low_words = _mm_and_si128(product_low_words, low_byte_mask);

				const auto inverted_mask_and_x = _mm_andnot_si128(low_byte_mask, as<__m128i>(x));
				auto widened_byte_products = _mm_maddubs_epi16(as<__m128i>(y), inverted_mask_and_x);

				return as<V>(_mm_or_si128(product_low_words, _mm_slli_epi16(widened_byte_products, 8)));
			}
			else {
				const auto left_odd = _mm_srli_epi16(as<__m128i>(x), 8);
				const auto right_odd = _mm_srli_epi16(as<__m128i>(y), 8);

				const auto multiplied_even = _mm_mullo_epi16(as<__m128i>(x), as<__m128i>(y));
				const auto multiplied_odd = _mm_slli_epi16(_mm_mullo_epi16(left_odd, right_odd), 8);

				return as<V>(select_<ISA,  T>(multiplied_even, multiplied_odd, _mm_set1_epi32(0x00FF00FF)));
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (ps<T>) return as<V>(_mm256_mul_ps(as<__m256>(x), as<__m256>(y)));
		else if constexpr (pd<T>) return as<V>(_mm256_mul_pd(as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (has_avx2<ISA>) {
			if constexpr (has_avx512bw<ISA> && has_avx512vl<ISA> && (epi8<T> || epu8<T>)) {
				const auto converted_x = _mm512_cvtepu8_epi16(as<__m256i>(x));
				const auto converted_y = _mm512_cvtepu8_epi16(as<__m256i>(y));

				const auto multiplied = _mm512_mullo_epi16(converted_x, converted_y);
				return as<V>(_mm512_cvtepi16_epi8(multiplied));
			}
			else if constexpr (epi8<T> || epu8<T>) {
				const auto low_byte_mask = _mm256_set1_epi16(0x00FF);
				const auto inverted_mask_and_vector_x = _mm256_andnot_si256(low_byte_mask, as<__m256i>(x));

				auto product_of_words = _mm256_mullo_epi16(as<__m256i>(y), as<__m256i>(x));
				product_of_words = _mm256_and_si256(product_of_words, low_byte_mask);

				auto widened_byte_products = _mm256_maddubs_epi16(as<__m256i>(y), inverted_mask_and_vector_x);
				widened_byte_products = _mm256_slli_epi16(widened_byte_products, 8);

				return as<V>(_mm256_or_si256(product_of_words, widened_byte_products));
			}
			else if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_mullo_epi16(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_mullo_epi32(as<__m256i>(x), as<__m256i>(y)));
			else if constexpr (epi64<T> || epu64<T>) {
				const auto x_shifted_high = _mm256_srli_epi64(as<__m256i>(x), 32);
				const auto y_shifted_high = _mm256_srli_epi64(as<__m256i>(y), 32);

				const auto product_low_low = _mm256_mul_epu32(as<__m256i>(x), as<__m256i>(y));
				const auto product_high_low = _mm256_mul_epu32(y_shifted_high, as<__m256i>(x));
				const auto product_low_high = _mm256_mul_epu32(x_shifted_high, as<__m256i>(y));

				const auto sum_cross_terms = _mm256_add_epi64(product_high_low, product_low_high);
				const auto shifted_cross_terms = _mm256_slli_epi64(sum_cross_terms, 32);

				return as<V>(_mm256_add_epi64(product_low_low, shifted_cross_terms));
			}
		}
		else {
			const auto low = mul_<arch::ISA::SSE42, T>(as<__m128i>(x), as<__m128i>(y));
			const auto high = mul_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1),
				_mm256_extractf128_si256(as<__m256i>(y), 1));

			return as<V>(_mm256_insertf128_si256(as<__m256i>(low), high, 1));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx512dq<ISA>) return as<V>(_mm512_mullo_epi64(as<__m512i>(x), as<__m512i>(y)));
			else {
				const auto left_high = _mm512_srli_epi64(as<__m512i>(x), 32);
				const auto right_high = _mm512_srli_epi64(as<__m512i>(y), 32);

				const auto multiplied1 = _mm512_mul_epu32(left_high, as<__m512i>(y));
				const auto multiplied2 = _mm512_mul_epu32(right_high, as<__m512i>(x));

				const auto add64 = _mm512_add_epi64(multiplied2, multiplied1);

				const auto high = _mm512_slli_epi64(add64, 32);
				const auto low = _mm512_mul_epu32(as<__m512i>(x), as<__m512i>(y));

				return as<V>(_mm512_add_epi64(low, high));
			}
		}
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mullo_epi32(as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_mul_ps(as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_mul_pd(as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) {
				return as<V>(_mm512_mullo_epi16(as<__m512i>(x), as<__m512i>(y)));
			}
			else if constexpr (epi8<T> || epu8<T>) {
				const auto mul1 = _mm512_mullo_epi16(as<__m512i>(x), as<__m512i>(y));
				const auto and_mask = _mm512_set1_epi32(0x00FF00FF);

				const auto 8bits = _mm512_andnot_si512(and_mask, as<__m512i>(x));
				const auto multiplied = _mm512_maddubs_epi16(as<__m512i>(y), 8bits);

				const auto shifted = _mm512_slli_epi16(multiplied, 8);
				return as<V>(_mm512_ternarylogic_epi64(shifted, mul1, and_mask, 0xF8));
			}
		}
		else {
			const auto low = mul_<arch::ISA::AVX2, T>(as<__m256i>(x), as<__m256i>(y));
			const auto high = mul_<arch::ISA::AVX2, T>(_mm512_extractf64x4_pd(as<__m512d>(x), 1),
				_mm512_extractf64x4_pd(as<__m512d>(y), 1));

			return as<V>(_mm512_inserti64x4(as<__m512i>(low), as<__m256i>(high), 1));
		}
	}
	else {
		return x * y;
	}
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type	M>
raze_nodiscard raze_always_inline V mul_(V x, V y, M mask) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (ps<T>) return as<V>(_mm_maskz_mul_ps(mask, as<__m128>(x), as<__m128>(y)));
		else if constexpr (pd<T>) return as<V>(_mm_maskz_mul_pd(mask, as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_maskz_mullo_epi32(mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (has_avx512dq<ISA> && (epi64<T> || epu64<T>)) {
			return as<V>(_mm_maskz_mullo_epi64(mask, as<__m128i>(x), as<__m128i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi16<T> || epu16<T>)) {
			return as<V>(_mm_maskz_mullo_epi16(mask, as<__m128i>(x), as<__m128i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi8<T> || epu8<T>)) {
			const auto converted_x = _mm256_cvtepu8_epi16(as<__m128i>(x));
			const auto converted_y = _mm256_cvtepu8_epi16(as<__m128i>(y));

			const auto multiplied = _mm256_mullo_epi16(converted_x, converted_y);
			return as<V>(_mm256_maskz_cvtepi16_epi8(mask, multiplied));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (ps<T>) return as<V>(_mm256_maskz_mul_ps(mask, as<__m256>(x), as<__m256>(y)));
		else if constexpr (pd<T>) return as<V>(_mm256_maskz_mul_pd(mask, as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_maskz_mullo_epi32(mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (has_avx512dq<ISA> && (epi64<T> || epu64<T>)) {
			return as<V>(_mm256_maskz_mullo_epi64(mask, as<__m256i>(x), as<__m256i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi16<T> || epu16<T>)) {
			return as<V>(_mm256_maskz_mullo_epi16(mask, as<__m256i>(x), as<__m256i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi8<T> || epu8<T>)) {
			const auto converted_x = _mm512_cvtepu8_epi16(as<__m256i>(x));
			const auto converted_y = _mm512_cvtepu8_epi16(as<__m256i>(y));

			const auto multiplied = _mm512_mullo_epi16(__converted_x, __converted_y);
			return as<V>(_mm512_maskz_cvtepi16_epi8(mask, __multiplied));
		}
	}
	else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
		if constexpr (ps<T>) return as<V>(_mm512_maskz_mul_ps(mask, as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_mul_pd(mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_mullo_epi32(mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (has_avx512dq<ISA> && (epi64<T> || epu64<T>)) {
			return as<V>(_mm512_maskz_mullo_epi64(mask, as<__m512i>(x), as<__m512i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi16<T> || epu16<T>)) {
			return as<V>(_mm512_maskz_mullo_epi16(mask, as<__m512i>(x), as<__m512i>(y)));
		}
	}

	return select_<ISA, T>(mul_<ISA, T>(x, y), mask);
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type	M>
raze_always_inline V mul_(V x, V y, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (ps<T>) return as<V>(_mm_mask_mul_ps(as<__m128>(src), mask, as<__m128>(x), as<__m128>(y)));
		else if constexpr (pd<T>) return as<V>(_mm_mask_mul_pd(as<__m128d>(src), mask, as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_mask_mullo_epi32(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		else if constexpr (has_avx512dq<ISA> && (epi64<T> || epu64<T>)) {
			return as<V>(_mm_mask_mullo_epi64(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi16<T> || epu16<T>)) {
			return as<V>(_mm_mask_mullo_epi16(as<__m128i>(src), mask, as<__m128i>(x), as<__m128i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi8<T> || epu8<T>)) {
			const auto converted_x = _mm256_cvtepu8_epi16(as<__m128i>(x));
			const auto converted_y = _mm256_cvtepu8_epi16(as<__m128i>(y));

			const auto multiplied = _mm256_mullo_epi16(converted_x, converted_y);
			return as<V>(_mm256_mask_cvtepi16_epi8(as<__m128i>(src), mask, multiplied));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && std::is_integral_v<M>) {
		if constexpr (ps<T>) return as<V>(_mm256_mask_mul_ps(as<__m256>(src), mask, as<__m256>(x), as<__m256>(y)));
		else if constexpr (pd<T>) return as<V>(_mm256_mask_mul_pd(as<__m256d>(src), mask, as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_mask_mullo_epi32(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		else if constexpr (has_avx512dq<ISA> && (epi64<T> || epu64<T>)) {
			return as<V>(_mm256_mask_mullo_epi64(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi16<T> || epu16<T>)) {
			return as<V>(_mm256_mask_mullo_epi16(as<__m256i>(src), mask, as<__m256i>(x), as<__m256i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi8<T> || epu8<T>)) {
			const auto converted_x = _mm512_cvtepu8_epi16(as<__m256i>(x));
			const auto converted_y = _mm512_cvtepu8_epi16(as<__m256i>(y));

			const auto multiplied = _mm512_mullo_epi16(converted_x, converted_y);
			return as<V>(_mm512_mask_cvtepi16_epi8(as<__m256i>(src), mask, multiplied));
		}
	}
	else if constexpr (sizeof(V) == 64 && std::is_integral_v<M>) {
		if constexpr (ps<T>) return as<V>(_mm512_mask_mul_ps(as<__m512>(src), mask, as<__m512>(x), as<__m512>(y)));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_mul_pd(as<__m512d>(src), mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_mullo_epi32(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		else if constexpr (has_avx512dq<ISA> && (epi64<T> || epu64<T>)) {
			return as<V>(_mm512_mask_mullo_epi64(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		}
		else if constexpr (has_avx512bw<ISA> && (epi16<T> || epu16<T>)) {
			return as<V>(_mm512_mask_mullo_epi16(as<__m512i>(src), mask, as<__m512i>(x), as<__m512i>(y)));
		}
	}

	return select_<ISA, T>(mul_<ISA, T>(x, y), src, mask);
}

__RAZE_VX_NAMESPACE_END
