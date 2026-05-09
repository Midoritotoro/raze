#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/arithmetic/Divisor.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_or_arithmetic_type _Tp_>
raze_nodiscard raze_always_inline _Tp_ __div_fallback(_Tp_ __x, _Tp_ __y) noexcept {
	if constexpr (arithmetic_type<_Tp_>) {
		return __x / __y;
	}
	else {
		constexpr auto __length = sizeof(_Tp_) / sizeof(_Type_);

		alignas(sizeof(_Tp_)) _Type_ __destination[__length];

		alignas(sizeof(_Tp_)) _Type_ __left_array[__length];
		alignas(sizeof(_Tp_)) _Type_ __right_array[__length];

		_Store<_ISA_>()(__left_array, __x, __aligned_policy{});
		_Store<_ISA_>()(__right_array, __y, __aligned_policy{});

		for (auto __i = 0; __i < __length; ++__i)
			__destination[__i] = __left_array[__i] / __right_array[__i];

		return _Load<_ISA_, _Tp_>()(__destination, __aligned_policy{});
	}
}

template <arch::ISA	_ISA_, arithmetic_type _Type_, intrin_type _Tp_>
raze_nodiscard raze_always_inline _Tp_ __div_fallback(_Tp_ __x, _Type_ __y) noexcept {
	if constexpr (arithmetic_type<_Tp_>) {
		return __x / __y;
	}
	else {
		constexpr auto __length = sizeof(_Tp_) / sizeof(_Type_);

		alignas(sizeof(_Tp_)) _Type_ __destination[__length];
		alignas(sizeof(_Tp_)) _Type_ __left_array[__length];

		_Store<_ISA_>()(__left_array, __x, __aligned_policy{});

		for (auto __i = 0; __i < __length; ++__i)
			__destination[__i] = __left_array[__i] / __y;

		return _Load<_ISA_, _Tp_>()(__destination, __aligned_policy{});
	}
}

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Div {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_div_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_div_ps(__as<__m128>(__x), __as<__m128>(__y)));
			else if constexpr (__has_avx_support_v<_ISA_> && __is_epi32_v<_Type_>) {
				return __as<_Tp_>(_mm256_cvttpd_epi32(_mm256_div_pd(_mm256_cvtepi32_pd(
					__as<__m128i>(__x)), _mm256_cvtepi32_pd(__as<__m128i>(__y)))));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_div_pd(__as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_div_ps(__as<__m256>(__x), __as<__m256>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_div_pd(__as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_div_ps(__as<__m512>(__x), __as<__m512>(__y)));
		}

		return __div_fallback<_ISA_, _Type_>(__x, __y);
	}

	template <intrin_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Type_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				const auto __left_0 = _mm_cvtsi128_si64(__as<__m128i>(__x));
				const auto __left_1 = _mm_cvtsi128_si64(__as<__m128i>(_mm_movehl_ps(
					__as<__m128>(__x), __as<__m128>(__x))));

				return _mm_set_epi64x(__left_1 / __y, __left_0 / __y);
			}
			else if constexpr (__is_epi32_v<_Type_>) {
				const auto __divisor_information = _Divisor<i32>(__y);
				const auto __low_product = _mm_mul_epu32(__as<__m128i>(__x), __divisor_information.multiplier());

				const auto __low_product_shifted = _mm_srli_epi64(__low_product, 32);
				const auto __left_value_shifted = _mm_srli_epi64(__as<__m128i>(__x), 32);

				const auto __high_product = _mm_mul_epu32(__left_value_shifted, __divisor_information.multiplier());
				const auto __low_32bit_mask = _mm_set_epi32(-1, 0, -1, 0);

				const auto __high_product_low_half = _mm_and_si128(__high_product, __low_32bit_mask);
				const auto __merged_product = _mm_or_si128(__low_product_shifted, __high_product_low_half);

				const auto __left_sign_mask = _mm_srai_epi32(__as<__m128i>(__x), 31);
				const auto __multiplier_sign_mask = _mm_srai_epi32(__divisor_information.multiplier(), 31);

				const auto __correction_from_multiplier = _mm_and_si128(__divisor_information.multiplier(), __left_sign_mask);
				const auto __correction_from_left_value = _mm_and_si128(__as<__m128i>(__x), __multiplier_sign_mask);

				const auto __correction_total = _mm_add_epi32(__correction_from_multiplier, __correction_from_left_value);
				const auto __adjusted_product = _mm_sub_epi32(__merged_product, __correction_total);

				const auto __biased_product = _mm_add_epi32(__adjusted_product, __as<__m128i>(__x));
				const auto __shifted_product = _mm_sra_epi32(__biased_product, __divisor_information.shift());

				const auto __sign_correction = _mm_sub_epi32(__left_sign_mask, __divisor_information.sign());
				const auto __quotient_value = _mm_sub_epi32(__shifted_product, __sign_correction);

				return __as<_Tp_>(_mm_xor_si128(__quotient_value, __divisor_information.sign()));
			}
			else if constexpr (__is_epu32_v<_Type_>) {
				const auto __divisor_information = _Divisor<u32>(__y);

				const auto __low_product = _mm_mul_epu32(__as<__m128i>(__x), __divisor_information.multiplier());
				const auto __low_product_shifted = _mm_srli_epi64(__low_product, 32);
				const auto __left_value_shifted = _mm_srli_epi64(__as<__m128i>(__x), 32);

				const auto __high_product = _mm_mul_epu32(__left_value_shifted, __divisor_information.multiplier());
				const auto __low_32bit_mask = _mm_set_epi32(-1, 0, -1, 0);

				const auto __high_product_low_half = _mm_and_si128(__high_product, __low_32bit_mask);
				const auto __merged_product = _mm_or_si128(__low_product_shifted, __high_product_low_half);

				const auto __difference_value = _mm_sub_epi32(__as<__m128i>(__x), __merged_product);
				const auto __first_shifted_value = _mm_srl_epi32(__difference_value, __divisor_information.shift1());
				const auto __biased_value = _mm_add_epi32(__merged_product, __first_shifted_value);

				return __as<_Tp_>(_mm_srl_epi32(__biased_value, __divisor_information.shift2()));
			}
			else if constexpr (__is_epi16_v<_Type_>) {
				const auto __divisor_information = _Divisor<i16>(__y);

				const auto __high_half_product = _mm_mulhi_epi16(__as<__m128i>(__x), __divisor_information.multiplier());
				const auto __sum_with_left_value = _mm_add_epi16(__high_half_product, __as<__m128i>(__x));

				const auto __shifted_sum = _mm_sra_epi16(__sum_with_left_value, __divisor_information.shift());
				const auto __left_sign_mask = _mm_srai_epi16(__as<__m128i>(__x), 15);
				const auto __sign_correction = _mm_sub_epi16(__left_sign_mask, __divisor_information.sign());
				const auto __adjusted_value = _mm_sub_epi16(__shifted_sum, __sign_correction);

				return __as<_Tp_>(_mm_xor_si128(__adjusted_value, __divisor_information.sign()));
			}
			else if constexpr (__is_epu16_v<_Type_>) {
				const auto __divisor_information = _Divisor<u16>(__y);

				const auto __high_half_product = _mm_mulhi_epu16(__as<__m128i>(__x), __divisor_information.multiplier());
				const auto __difference_value = _mm_sub_epi16(__as<__m128i>(__x), __high_half_product);

				const auto __first_shifted_value = _mm_srl_epi16(__difference_value, __divisor_information.shift1());
				const auto __biased_value = _mm_add_epi16(__high_half_product, __first_shifted_value);

				return __as<_Tp_>(_mm_srl_epi16(__biased_value, __divisor_information.shift2()));
			}
			else if constexpr (__is_epi8_v<_Type_>) {
				const auto __sign = _mm_cmpgt_epi8(_mm_setzero_si128(), __as<__m128i>(__x));
				const auto __mask = _mm_set1_epi32(0x00FF00FF);

				const auto __low = _Div<_ISA_, i16>()(_mm_unpacklo_epi8(__as<__m128i>(__x), __sign), __y);
				const auto __high = _Div<_ISA_, i16>()(_mm_unpackhi_epi8(__as<__m128i>(__x), __sign), __y);

				return __as<_Tp_>(_mm_packus_epi16(_mm_and_si128(__low, __mask), _mm_and_si128(__high, __mask)));
			}
			else if constexpr (__is_epu8_v<_Type_>) {
				const auto __mask = _mm_set1_epi32(0x00FF00FF);

				const auto __low_part = _mm_unpacklo_epi8(__as<__m128i>(__x), _mm_setzero_si128());
				const auto __high_part = _mm_unpackhi_epi8(__as<__m128i>(__x), _mm_setzero_si128());

				const auto __low = _Div<_ISA_, u16>()(__low_part, __y);
				const auto __high = _Div<_ISA_, u16>()(__high_part, __y);

				return __as<_Tp_>(_mm_packus_epi16(_mm_and_si128(__low, __mask), _mm_and_si128(__high, __mask)));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_div_ps(__as<__m256>(__x), __as<__m256>(__y)));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_div_pd(__as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__has_avx2_support_v<_ISA_>) {
				if constexpr (__is_epi32_v<_Type_>) {
					const auto __divisor_information = _Divisor<i32>(__y);

					const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
					const auto __sign_broadcast = _mm256_broadcastq_epi64(__divisor_information.sign());

					const auto __low_half_product = _mm256_mul_epi32(__as<__m256i>(__x), __multiplier_broadcast);
					const auto __low_half_product_shifted = _mm256_srli_epi64(__low_half_product, 32);

					const auto __left_value_shifted = _mm256_srli_epi64(__as<__m256i>(__x), 32);
					const auto __high_half_product = _mm256_mul_epi32(__left_value_shifted, __multiplier_broadcast);

					const auto __merged_product = _mm256_blend_epi32(__low_half_product_shifted, __high_half_product, 0xAA);

					const auto __biased_value = _mm256_add_epi32(__merged_product, __as<__m256i>(__x));
					const auto __shifted_value = _mm256_sra_epi32(__biased_value, __divisor_information.shift());

					const auto __left_sign_mask = _mm256_srai_epi32(__as<__m256i>(__x), 31);

					const auto __sign_correction = _mm256_sub_epi32(__left_sign_mask, __sign_broadcast);
					const auto __adjusted_value = _mm256_sub_epi32(__shifted_value, __sign_correction);

					return __as<_Tp_>(_mm256_xor_si256(__adjusted_value, __sign_broadcast));
				}
				else if constexpr (__is_epu32_v<_Type_>) {
					const auto __divisor_information = _Divisor<u32>(__y);

					const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
					const auto __low_half_product = _mm256_mul_epu32(__as<__m256i>(__x), __multiplier_broadcast);
					const auto __low_half_product_shifted = _mm256_srli_epi64(__low_half_product, 32);

					const auto __left_value_shifted = _mm256_srli_epi64(__as<__m256i>(__x), 32);
					const auto __high_half_product = _mm256_mul_epu32(__left_value_shifted, __multiplier_broadcast);

					const auto __merged_product = _mm256_blend_epi32(__low_half_product_shifted, __high_half_product, 0xAA);
					const auto __difference_value = _mm256_sub_epi32(__as<__m256i>(__x), __merged_product);

					const auto __first_shifted_value = _mm256_srl_epi32(__difference_value, __divisor_information.shift1());
					const auto __biased_value = _mm256_add_epi32(__merged_product, __first_shifted_value);

					return __as<_Tp_>(_mm256_srl_epi32(__biased_value, __divisor_information.shift2()));
				}
				else if constexpr (__is_epi16_v<_Type_>) {
					const auto __divisor_information = _Divisor<i16>(__y);

					const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
					const auto __sign_broadcast = _mm256_broadcastq_epi64(__divisor_information.sign());

					const auto __high_half_product = _mm256_mulhi_epi16(__as<__m256i>(__x), __multiplier_broadcast);
					const auto __sum_with_left_value = _mm256_add_epi16(__high_half_product, __as<__m256i>(__x));

					const auto __shifted_sum = _mm256_sra_epi16(__sum_with_left_value, __divisor_information.shift());
					const auto __left_sign_mask = _mm256_srai_epi16(__as<__m256i>(__x), 15);

					const auto __sign_correction = _mm256_sub_epi16(__left_sign_mask, __sign_broadcast);
					const auto __adjusted_value = _mm256_sub_epi16(__shifted_sum, __sign_correction);

					return __as<_Tp_>(_mm256_xor_si256(__adjusted_value, __sign_broadcast));
				}
				else if constexpr (__is_epu16_v<_Type_>) {
					const auto __divisor_information = _Divisor<u16>(__y);

					const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
					const auto __high_half_product = _mm256_mulhi_epu16(__as<__m256i>(__x), __multiplier_broadcast);

					const auto __difference_value = _mm256_sub_epi16(__as<__m256i>(__x), __high_half_product);
					const auto __first_shifted_value = _mm256_srl_epi16(__difference_value, __divisor_information.shift1());

					const auto __biased_value = _mm256_add_epi16(__high_half_product, __first_shifted_value);
					return __as<_Tp_>(_mm256_srl_epi16(__biased_value, __divisor_information.shift2()));
				}
				else if constexpr (__is_epi8_v<_Type_>) {
					const auto __zeros = _mm256_setzero_si256();
					const auto __and_mask = _mm256_set1_epi32(0x00FF00FF);

					const auto __shuffled_low = _mm256_permute4x64_epi64(__as<__m256i>(__x), 0x10);
					const auto __shuffled_high = _mm256_permute4x64_epi64(__as<__m256i>(__x), 0xC8);

					const auto __sign_low = _mm256_cmpgt_epi8(__zeros, __shuffled_low);
					const auto __sign_high = _mm256_cmpgt_epi8(__zeros, __shuffled_high);

					const auto __low = _mm256_unpacklo_epi8(__shuffled_low, __sign_low);
					const auto __high = _mm256_unpackhi_epi8(__shuffled_high, __sign_high);

					const auto __divided_low = _Div<arch::ISA::AVX2, i16>()(__low, __y);
					const auto __divided_high = _Div<arch::ISA::AVX2, i16>()(__high, __y);

					const auto __low_mask = _mm256_and_si256(__divided_low, __and_mask);
					const auto __high_mask = _mm256_and_si256(__divided_high, __and_mask);

					return __as<_Tp_>(_mm256_permute4x64_epi64(_mm256_packus_epi16(__low_mask, __high_mask), 0xD8));
				}
				else if constexpr (__is_epu8_v<_Type_>) {
					const auto __zeros = _mm256_setzero_si256();
					const auto __and_mask = _mm256_set1_epi32(0x00FF00FF);

					const auto __shuffled_low = _mm256_permute4x64_epi64(__as<__m256i>(__x), 0x10);
					const auto __shuffled_high = _mm256_permute4x64_epi64(__as<__m256i>(__x), 0xC8);

					const auto __low = _mm256_unpacklo_epi8(__shuffled_low, __zeros);
					const auto __high = _mm256_unpackhi_epi8(__shuffled_high, __zeros);

					const auto __divided_low = _Div<arch::ISA::AVX2, i16>()(__low, __y);
					const auto __divided_high = _Div<arch::ISA::AVX2, i16>()(__high, __y);

					const auto __low_mask = _mm256_and_si256(__divided_low, __and_mask);
					const auto __high_mask = _mm256_and_si256(__divided_high, __and_mask);

					return __as<_Tp_>(_mm256_permute4x64_epi64(_mm256_packus_epi16(__low_mask, __high_mask), 0xD8));
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi32_v<_Type_>) {
				const auto __divisor_information = _Divisor<i32>(__y);

				const auto __multiplier_broadcast = _mm512_broadcast_i32x4(__divisor_information.multiplier());
				const auto __sign_broadcast = _mm512_broadcast_i32x4(__divisor_information.sign());

				const auto __low_half_product = _mm512_mul_epi32(__as<__m512i>(__x), __multiplier_broadcast);
				const auto __left_shifted_high = _mm512_srli_epi64(__as<__m512i>(__x), 32);

				const auto __high_half_product = _mm512_mul_epi32(__left_shifted_high, __multiplier_broadcast);
				const auto __low_half_product_shifted = _mm512_srli_epi64(__low_half_product, 32);

				const auto __merged_product = _mm512_mask_mov_epi32(__low_half_product_shifted, 0xAAAA, __high_half_product);
				const auto __biased_value = _mm512_add_epi32(__merged_product, __as<__m512i>(__x));

				const auto __shifted_value = _mm512_sra_epi32(__biased_value, __divisor_information.shift());
				const auto __left_sign_mask = _mm512_srai_epi32(__as<__m512i>(__x), 31);

				const auto __sign_correction = _mm512_sub_epi32(__left_sign_mask, __sign_broadcast);
				const auto __adjusted_value = _mm512_sub_epi32(__shifted_value, __sign_correction);

				return __as<_Tp_>(_mm512_xor_si512(__adjusted_value, __sign_broadcast));
			}
			else if constexpr (__is_epu32_v<_Type_>) {
				const auto __divisor_information = _Divisor<u32>(__y);

				const auto __multiplier_broadcast = _mm512_broadcast_i32x4(__divisor_information.multiplier());
				const auto __low_half_product = _mm512_mul_epu32(__as<__m512i>(__x), __multiplier_broadcast);

				const auto __left_shifted_high = _mm512_srli_epi64(__as<__m512i>(__x), 32);
				const auto __high_half_product = _mm512_mul_epu32(__left_shifted_high, __multiplier_broadcast);

				const auto __low_half_product_shifted = _mm512_srli_epi64(__low_half_product, 32);
				const auto __merged_product = _mm512_mask_mov_epi32(__low_half_product_shifted, 0xAAAA, __high_half_product);
				const auto __difference_value = _mm512_sub_epi32(__as<__m512i>(__x), __merged_product);

				const auto __first_shifted_value = _mm512_srl_epi32(__difference_value, __divisor_information.shift1());
				const auto __biased_value = _mm512_add_epi32(__merged_product, __first_shifted_value);

				return _mm512_srl_epi32(__biased_value, __divisor_information.shift2());
			}
		}

		return (*this)(__x, _Broadcast<_ISA_, _Tp_>()(__y));
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_maskz_div_pd(__mask, __as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_maskz_div_ps(__mask, __as<__m128>(__x), __as<__m128>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_maskz_div_pd(__mask, __as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_maskz_div_ps(__mask, __as<__m256>(__x), __as<__m256>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_div_pd(__mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_div_ps(__mask, __as<__m512>(__x), __as<__m512>(__y)));
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Type_ __y, _Mask_ __mask) const noexcept {
		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mask_div_pd(__as<__m128d>(__src), __mask, __as<__m128d>(__x), __as<__m128d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mask_div_ps(__as<__m128>(__src), __mask, __as<__m128>(__x), __as<__m128>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_>) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mask_div_pd(__as<__m256d>(__src), __mask, __as<__m256d>(__x), __as<__m256d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mask_div_ps(__as<__m256>(__src), __mask, __as<__m256>(__x), __as<__m256>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_div_pd(__as<__m512d>(__src), __mask, __as<__m512d>(__x), __as<__m512d>(__y)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_div_ps(__as<__m512>(__src), __mask, __as<__m512>(__x), __as<__m512>(__y)));
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __src, __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Type_ __y, _Mask_ __mask, _Tp_ __src) const noexcept {
		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
