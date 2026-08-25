#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/arithmetic/Divisor.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V div_fallback_(V x, V y) noexcept {
	if constexpr (arithmetic_type<V>) {
		return x / y;
	}
	else {
		constexpr auto size = sizeof(V) / sizeof(T);

		alignas(sizeof(V)) T dest[size];

		alignas(sizeof(V)) T left[size];
		alignas(sizeof(V)) T right[size];

		store_(left, x, aligned_policy{});
		store_(right, y, aligned_policy{});

		for (auto i = 0; i < size; ++i)
			dest[i] = left[i] / right[i];

		return load_<ISA, V>(dest, aligned_policy{});
	}
}

template <arch::ISA	ISA, arithmetic_type T, intrin_type V>
raze_nodiscard raze_always_inline V div_fallback_(V x, T y) noexcept {
	if constexpr (arithmetic_type<V>) {
		return x / y;
	}
	else {
		constexpr auto size = sizeof(V) / sizeof(T);

		alignas(sizeof(V)) T dest[__length];
		alignas(sizeof(V)) T left[__length];

		store_(left, x, aligned_policy{});

		for (auto i = 0; i < size; ++i)
			dest[i] = left[i] / y;

		return load_<ISA, V>(dest, aligned_policy{});
	}
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline V div_(V x, V y) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (pd<T>) return as<V>(_mm_div_pd(as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_div_ps(as<__m128>(x), as<__m128>(y)));
		else if constexpr (has_avx<ISA> && epi32<T>) {
			return as<V>(_mm256_cvttpd_epi32(_mm256_div_pd(_mm256_cvtepi32_pd(
				as<__m128i>(x)), _mm256_cvtepi32_pd(as<__m128i>(y)))));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (pd<T>) return as<_Tp_>(_mm256_div_pd(as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (ps<T>) return as<_Tp_>(_mm256_div_ps(as<__m256>(x), as<__m256>(y)));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (pd<T>) return as<_Tp_>(_mm512_div_pd(as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (ps<T>) return as<_Tp_>(_mm512_div_ps(as<__m512>(x), as<__m512>(y)));
	}

	return div_fallback_<ISA, T>(x, y);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_type V>
raze_always_inline V div_(V x, T y) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T> || epu64<T>) {
			const auto left_0 = _mm_cvtsi128_si64(as<__m128i>(x));
			const auto left_1 = _mm_cvtsi128_si64(as<__m128i>(_mm_movehl_ps(
				as<__m128>(x), as<__m128>(x))));

			return _mm_set_epi64x(left_1 / y, left_0 / y);
		}
		else if constexpr (epi32<T>) {
			const auto divisor_information = vector_divisor<i32>(y);
			const auto low_product = _mm_mul_epu32(as<__m128i>(x), divisor_information.multiplier());

			const auto low_product_shifted = _mm_srli_epi64(low_product, 32);
			const auto left_value_shifted = _mm_srli_epi64(as<__m128i>(x), 32);

			const auto high_product = _mm_mul_epu32(left_value_shifted, divisor_information.multiplier());
			const auto low_32bit_mask = _mm_set_epi32(-1, 0, -1, 0);

			const auto high_product_low_half = _mm_and_si128(high_product, low_32bit_mask);
			const auto merged_product = _mm_or_si128(low_product_shifted, high_product_low_half);

			const auto left_sign_mask = _mm_srai_epi32(as<__m128i>(x), 31);
			const auto multiplier_sign_mask = _mm_srai_epi32(divisor_information.multiplier(), 31);

			const auto correction_from_multiplier = _mm_and_si128(divisor_information.multiplier(), left_sign_mask);
			const auto correction_from_left_value = _mm_and_si128(as<__m128i>(x), multiplier_sign_mask);

			const auto correction_total = _mm_add_epi32(correction_from_multiplier, correction_from_left_value);
			const auto adjusted_product = _mm_sub_epi32(merged_product, correction_total);

			const auto biased_product = _mm_add_epi32(adjusted_product, as<__m128i>(x));
			const auto shifted_product = _mm_sra_epi32(biased_product, divisor_information.shift());

			const auto sign_correction = _mm_sub_epi32(left_sign_mask, divisor_information.sign());
			const auto quotient_value = _mm_sub_epi32(shifted_product, sign_correction);

			return as<V>(_mm_xor_si128(quotient_value, divisor_information.sign()));
		}
		else if constexpr (epu32<T>) {
			const auto divisor_information = vector_divisor<u32>(y);

			const auto low_product = _mm_mul_epu32(as<__m128i>(__x), divisor_information.multiplier());
			const auto low_product_shifted = _mm_srli_epi64(low_product, 32);
			const auto left_value_shifted = _mm_srli_epi64(as<__m128i>(x), 32);

			const auto high_product = _mm_mul_epu32(left_value_shifted, divisor_information.multiplier());
			const auto low_32bit_mask = _mm_set_epi32(-1, 0, -1, 0);

			const auto high_product_low_half = _mm_and_si128(high_product, low_32bit_mask);
			const auto merged_product = _mm_or_si128(low_product_shifted, high_product_low_half);

			const auto difference_value = _mm_sub_epi32(as<__m128i>(x), merged_product);
			const auto first_shifted_value = _mm_srl_epi32(difference_value, divisor_information.shift1());
			const auto biased_value = _mm_add_epi32(merged_product, first_shifted_value);

			return as<V>(_mm_srl_epi32(biased_value, divisor_information.shift2()));
		}
		else if constexpr (epi16<T>) {
			const auto divisor_information = vector_divisor<i16>(y);

			const auto high_half_product = _mm_mulhi_epi16(as<__m128i>(x), divisor_information.multiplier());
			const auto sum_with_left_value = _mm_add_epi16(high_half_product, as<__m128i>(x));

			const auto shifted_sum = _mm_sra_epi16(sum_with_left_value, divisor_information.shift());
			const auto left_sign_mask = _mm_srai_epi16(as<__m128i>(x), 15);
			const auto sign_correction = _mm_sub_epi16(left_sign_mask, divisor_information.sign());
			const auto adjusted_value = _mm_sub_epi16(shifted_sum, sign_correction);

			return as<V>(_mm_xor_si128(adjusted_value, divisor_information.sign()));
		}
		else if constexpr (epu16<T>) {
			const auto divisor_information = vector_divisor<u16>(__y);

			const auto high_half_product = _mm_mulhi_epu16(as<__m128i>(x), divisor_information.multiplier());
			const auto difference_value = _mm_sub_epi16(as<__m128i>(x), high_half_product);

			const auto first_shifted_value = _mm_srl_epi16(difference_value, divisor_information.shift1());
			const auto biased_value = _mm_add_epi16(high_half_product, first_shifted_value);

			return as<V>(_mm_srl_epi16(biased_value, divisor_information.shift2()));
		}
		else if constexpr (epi8<T>) {
			const auto sign = _mm_cmpgt_epi8(_mm_setzero_si128(), as<__m128i>(x));
			const auto mask = _mm_set1_epi32(0x00FF00FF);

			const auto low = div_<ISA, i16>(_mm_unpacklo_epi8(as<__m128i>(x), sign), y);
			const auto high = div_<ISA, i16>(_mm_unpackhi_epi8(as<__m128i>(x), sign), y);

			return as<V>(_mm_packus_epi16(_mm_and_si128(low, mask), _mm_and_si128(high, mask)));
		}
		else if constexpr (epu8<T>) {
			const auto mask = _mm_set1_epi32(0x00FF00FF);

			const auto low_part = _mm_unpacklo_epi8(as<__m128i>(x), _mm_setzero_si128());
			const auto high_part = _mm_unpackhi_epi8(as<__m128i>(x), _mm_setzero_si128());

			const auto low = div_<ISA, u16>()(low_part, y);
			const auto high = div_<ISA, u16>()(high_part, y);

			return as<V>(_mm_packus_epi16(_mm_and_si128(low, mask), _mm_and_si128(high, mask)));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx2<ISA>) {
			if constexpr (epi32<T>) {
				const auto divisor_information = vector_divisor<i32>(__y);

				const auto multiplier_broadcast = _mm256_broadcastq_epi64(divisor_information.multiplier());
				const auto sign_broadcast = _mm256_broadcastq_epi64(divisor_information.sign());

				const auto low_half_product = _mm256_mul_epi32(as<__m256i>(x), multiplier_broadcast);
				const auto low_half_product_shifted = _mm256_srli_epi64(low_half_product, 32);

				const auto left_value_shifted = _mm256_srli_epi64(as<__m256i>(x), 32);
				const auto high_half_product = _mm256_mul_epi32(left_value_shifted, multiplier_broadcast);

				const auto merged_product = _mm256_blend_epi32(low_half_product_shifted, high_half_product, 0xAA);

				const auto biased_value = _mm256_add_epi32(merged_product, as<__m256i>(x));
				const auto shifted_value = _mm256_sra_epi32(biased_value, divisor_information.shift());

				const auto left_sign_mask = _mm256_srai_epi32(as<__m256i>(x), 31);

				const auto sign_correction = _mm256_sub_epi32(left_sign_mask, sign_broadcast);
				const auto adjusted_value = _mm256_sub_epi32(shifted_value, sign_correction);

				return as<V>(_mm256_xor_si256(adjusted_value, sign_broadcast));
			}
			else if constexpr (epu32<T>) {
				const auto divisor_information = vector_divisor<u32>(y);

				const auto multiplier_broadcast = _mm256_broadcastq_epi64(divisor_information.multiplier());
				const auto low_half_product = _mm256_mul_epu32(as<__m256i>(x), multiplier_broadcast);
				const auto low_half_product_shifted = _mm256_srli_epi64(low_half_product, 32);

				const auto left_value_shifted = _mm256_srli_epi64(as<__m256i>(x), 32);
				const auto high_half_product = _mm256_mul_epu32(left_value_shifted, multiplier_broadcast);

				const auto merged_product = _mm256_blend_epi32(low_half_product_shifted, high_half_product, 0xAA);
				const auto difference_value = _mm256_sub_epi32(as<__m256i>(x), merged_product);

				const auto first_shifted_value = _mm256_srl_epi32(difference_value, divisor_information.shift1());
				const auto biased_value = _mm256_add_epi32(merged_product, first_shifted_value);

				return as<V>(_mm256_srl_epi32(biased_value, divisor_information.shift2()));
			}
			else if constexpr (epi16<T>) {
				const auto divisor_information = vector_divisor<i16>(y);

				const auto multiplier_broadcast = _mm256_broadcastq_epi64(divisor_information.multiplier());
				const auto sign_broadcast = _mm256_broadcastq_epi64(divisor_information.sign());

				const auto high_half_product = _mm256_mulhi_epi16(as<__m256i>(x), multiplier_broadcast);
				const auto sum_with_left_value = _mm256_add_epi16(high_half_product, as<__m256i>(x));

				const auto shifted_sum = _mm256_sra_epi16(sum_with_left_value, divisor_information.shift());
				const auto left_sign_mask = _mm256_srai_epi16(as<__m256i>(x), 15);

				const auto sign_correction = _mm256_sub_epi16(left_sign_mask, sign_broadcast);
				const auto adjusted_value = _mm256_sub_epi16(shifted_sum, sign_correction);

				return as<V>(_mm256_xor_si256(adjusted_value, sign_broadcast));
			}
			else if constexpr (epu16<T>) {
				const auto divisor_information = vector_divisor<u16>(y);

				const auto multiplier_broadcast = _mm256_broadcastq_epi64(divisor_information.multiplier());
				const auto high_half_product = _mm256_mulhi_epu16(as<__m256i>(x), multiplier_broadcast);

				const auto difference_value = _mm256_sub_epi16(as<__m256i>(x), high_half_product);
				const auto first_shifted_value = _mm256_srl_epi16(difference_value, divisor_information.shift1());

				const auto biased_value = _mm256_add_epi16(high_half_product, first_shifted_value);
				return as<V>(_mm256_srl_epi16(biased_value, divisor_information.shift2()));
			}
			else if constexpr (epi8<T>) {
				const auto zeros = _mm256_setzero_si256();
				const auto and_mask = _mm256_set1_epi32(0x00FF00FF);

				const auto shuffled_low = _mm256_permute4x64_epi64(as<__m256i>(x), 0x10);
				const auto shuffled_high = _mm256_permute4x64_epi64(as<__m256i>(x), 0xC8);

				const auto sign_low = _mm256_cmpgt_epi8(zeros, shuffled_low);
				const auto sign_high = _mm256_cmpgt_epi8(zeros, shuffled_high);

				const auto low = _mm256_unpacklo_epi8(shuffled_low, sign_low);
				const auto high = _mm256_unpackhi_epi8(shuffled_high, sign_high);

				const auto divided_low = div_<arch::ISA::AVX2, i16>(low, y);
				const auto divided_high = div_<arch::ISA::AVX2, i16>(high, y);

				const auto low_mask = _mm256_and_si256(divided_low, and_mask);
				const auto high_mask = _mm256_and_si256(divided_high, and_mask);

				return as<V>(_mm256_permute4x64_epi64(_mm256_packus_epi16(low_mask,_high_mask), 0xD8));
			}
			else if constexpr (epu8<T>) {
				const auto zeros = _mm256_setzero_si256();
				const auto and_mask = _mm256_set1_epi32(0x00FF00FF);

				const auto shuffled_low = _mm256_permute4x64_epi64(as<__m256i>(x), 0x10);
				const auto shuffled_high = _mm256_permute4x64_epi64(as<__m256i>(x), 0xC8);

				const auto low = _mm256_unpacklo_epi8(shuffled_low, zeros);
				const auto high = _mm256_unpackhi_epi8(shuffled_high, zeros);

				const auto divided_low = div_<arch::ISA::AVX2, i16>(low, y);
				const auto divided_high = div_<arch::ISA::AVX2, i16>(high, y);

				const auto low_mask = _mm256_and_si256(divided_low, and_mask);
				const auto high_mask = _mm256_and_si256(divided_high, and_mask);

				return as<V>(_mm256_permute4x64_epi64(_mm256_packus_epi16(low_mask, high_mask), 0xD8));
			}
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi32<T>) {
			const auto divisor_information = vector_divisor<i32>(y);

			const auto multiplier_broadcast = _mm512_broadcast_i32x4(divisor_information.multiplier());
			const auto sign_broadcast = _mm512_broadcast_i32x4(divisor_information.sign());

			const auto low_half_product = _mm512_mul_epi32(as<__m512i>(x), multiplier_broadcast);
			const auto left_shifted_high = _mm512_srli_epi64(as<__m512i>(x), 32);

			const auto high_half_product = _mm512_mul_epi32(left_shifted_high, multiplier_broadcast);
			const auto low_half_product_shifted = _mm512_srli_epi64(low_half_product, 32);

			const auto merged_product = _mm512_mask_mov_epi32(low_half_product_shifted, 0xAAAA, high_half_product);
			const auto biased_value = _mm512_add_epi32(merged_product, as<__m512i>(x));

			const auto shifted_value = _mm512_sra_epi32(biased_value, divisor_information.shift());
			const auto left_sign_mask = _mm512_srai_epi32(as<__m512i>(x), 31);

			const auto sign_correction = _mm512_sub_epi32(left_sign_mask, sign_broadcast);
			const auto adjusted_value = _mm512_sub_epi32(shifted_value, sign_correction);

			return as<V>(_mm512_xor_si512(adjusted_value, sign_broadcast));
		}
		else if constexpr (epu32<T>) {
			const auto divisor_information = vector_divisor<u32>(y);

			const auto multiplier_broadcast = _mm512_broadcast_i32x4(divisor_information.multiplier());
			const auto low_half_product = _mm512_mul_epu32(as<__m512i>(x), multiplier_broadcast);

			const auto left_shifted_high = _mm512_srli_epi64(as<__m512i>(x), 32);
			const auto high_half_product = _mm512_mul_epu32(left_shifted_high, multiplier_broadcast);

			const auto low_half_product_shifted = _mm512_srli_epi64(low_half_product, 32);
			const auto merged_product = _mm512_mask_mov_epi32(low_half_product_shifted, 0xAAAA, high_half_product);
			const auto difference_value = _mm512_sub_epi32(as<__m512i>(x), merged_product);

			const auto first_shifted_value = _mm512_srl_epi32(difference_value, divisor_information.shift1());
			const auto biased_value = _mm512_add_epi32(merged_product, first_shifted_value);

			return as<V>(_mm512_srl_epi32(biased_value, divisor_information.shift2()));
		}
	}

	return div_<ISA, T>(x, _Broadcast<ISA, V>()(y));
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V div_(V x, V y, M mask) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (pd<T>) return as<V>(_mm_maskz_div_pd(mask, as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_maskz_div_ps(mask, as<__m128>(x), as<__m128>(y)));
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (pd<T>) return as<V>(_mm256_maskz_div_pd(mask, as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm256_maskz_div_ps(mask, as<__m256>(x), as<__m256>(y)));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (pd<T>) return as<V>(_mm512_maskz_div_pd(mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_div_ps(mask, as<__m512>(x), as<__m512>(y)));
	}

	return select_<ISA, T>(div_<ISA, T>(x, y), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V div_(V x, T y, M mask) noexcept {
	return select_<ISA, T>(div_<ISA, T>(x, y), mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V div_(V x, V y, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (pd<T>) return as<V>(_mm_mask_div_pd(as<__m128d>(src), mask, as<__m128d>(x), as<__m128d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm_mask_div_ps(as<__m128>(src), mask, as<__m128>(x), as<__m128>(y)));
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (pd<T>) return as<V>(_mm256_mask_div_pd(as<__m256d>(src), mask, as<__m256d>(x), as<__m256d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm256_mask_div_ps(as<__m256>(src), mask, as<__m256>(x), as<__m256>(y)));
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (pd<T>) return as<V>(_mm512_mask_div_pd(as<__m512d>(src), mask, as<__m512d>(x), as<__m512d>(y)));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_div_ps(as<__m512>(src), mask, as<__m512>(x), as<__m512>(y)));
	}

	return select_<ISA, T>(div_<ISA, T>(x, y), src, mask);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline V div_(V x, T y, M mask, V src) noexcept {
	return select_<ISA, T>(div_<ISA, T>(x, y), src, mask);
}

__RAZE_VX_NAMESPACE_END
