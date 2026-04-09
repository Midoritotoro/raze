#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/Blend.h>
#include <src/raze/datapar/arithmetic/Divisor.h>
#include <src/raze/datapar/memory/Store.h>
#include <src/raze/datapar/memory/Load.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Div;

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_,
	class		_IntrinType_>
raze_nodiscard raze_always_inline _IntrinType_ __div_fallback(
	_IntrinType_ __left,
	_IntrinType_ __right) noexcept
{
	constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);
	
	alignas(sizeof(_IntrinType_)) _DesiredType_ __destination[__length];

	alignas(sizeof(_IntrinType_)) _DesiredType_ __left_array[__length];
	alignas(sizeof(_IntrinType_)) _DesiredType_ __right_array[__length];

	_Store<_ISA_, _Width_>()(__left_array, __left, __aligned_policy{});
	_Store<_ISA_, _Width_>()(__right_array, __right, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__destination[__i] = __left_array[__i] / __right_array[__i];

	return _Load<_ISA_, _Width_, _IntrinType_>()(__destination, __aligned_policy{});
}

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_,
	class		_IntrinType_>
raze_nodiscard raze_always_inline _IntrinType_ __div_fallback(
	_IntrinType_	__left,
	_DesiredType_	__right) noexcept
{
	constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);

	alignas(sizeof(_IntrinType_)) _DesiredType_ __destination[__length];
	alignas(sizeof(_IntrinType_)) _DesiredType_ __left_array[__length];

	_Store<_ISA_, _Width_>()(__left_array, __left, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__destination[__i] = __left_array[__i] / __right;

	return _Load<_ISA_, _Width_, _IntrinType_>()(__destination, __aligned_policy{});
}

template <class _DesiredType_>
struct _Div<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right) raze_const_operator noexcept
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_div_pd(
				__intrin_bitcast<__m128d>(__left), _mm_set1_pd(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_div_ps(
				__intrin_bitcast<__m128>(__left), _mm_set1_ps(__right)));
		}
		else if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			const auto __left_0 = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__left));
			const auto __left_1 = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(_mm_movehl_ps(
				__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__left))));

			return _mm_set_epi64x(__left_1 / __right, __left_0 / __right);
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<int32>(__right);

			const auto __low_product = _mm_mul_epu32(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());

			const auto __low_product_shifted = _mm_srli_epi64(__low_product, 32);
			const auto __left_value_shifted = _mm_srli_epi64(__intrin_bitcast<__m128i>(__left), 32);

			const auto __high_product = _mm_mul_epu32(__left_value_shifted, __divisor_information.multiplier());
			const auto __low_32bit_mask = _mm_set_epi32(-1, 0, -1, 0);

			const auto __high_product_low_half = _mm_and_si128(__high_product, __low_32bit_mask);
			const auto __merged_product = _mm_or_si128(__low_product_shifted, __high_product_low_half);

			const auto __left_sign_mask = _mm_srai_epi32(__intrin_bitcast<__m128i>(__left), 31);
			const auto __multiplier_sign_mask = _mm_srai_epi32(__divisor_information.multiplier(), 31);

			const auto __correction_from_multiplier = _mm_and_si128(__divisor_information.multiplier(), __left_sign_mask);
			const auto __correction_from_left_value = _mm_and_si128(__intrin_bitcast<__m128i>(__left), __multiplier_sign_mask);

			const auto __correction_total = _mm_add_epi32(__correction_from_multiplier, __correction_from_left_value);
			const auto __adjusted_product = _mm_sub_epi32(__merged_product, __correction_total);

			const auto __biased_product = _mm_add_epi32(__adjusted_product, __intrin_bitcast<__m128i>(__left));
			const auto __shifted_product = _mm_sra_epi32(__biased_product, __divisor_information.shift());

			const auto __sign_correction = _mm_sub_epi32(__left_sign_mask, __divisor_information.sign());
			const auto __quotient_value = _mm_sub_epi32(__shifted_product, __sign_correction);

			return __intrin_bitcast<_IntrinType_>(_mm_xor_si128(__quotient_value, __divisor_information.sign()));
		}
		else if constexpr (__is_epu32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<uint32>(__right);

			const auto __low_product = _mm_mul_epu32(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());
			const auto __low_product_shifted = _mm_srli_epi64(__low_product, 32);
			const auto __left_value_shifted = _mm_srli_epi64(__intrin_bitcast<__m128i>(__left), 32);

			const auto __high_product = _mm_mul_epu32(__left_value_shifted, __divisor_information.multiplier());
			const auto __low_32bit_mask = _mm_set_epi32(-1, 0, -1, 0);

			const auto __high_product_low_half = _mm_and_si128(__high_product, __low_32bit_mask);
			const auto __merged_product = _mm_or_si128(__low_product_shifted, __high_product_low_half);

			const auto __difference_value = _mm_sub_epi32(__intrin_bitcast<__m128i>(__left), __merged_product);
			const auto __first_shifted_value = _mm_srl_epi32(__difference_value, __divisor_information.shift1());
			const auto __biased_value = _mm_add_epi32(__merged_product, __first_shifted_value);

			return __intrin_bitcast<_IntrinType_>(_mm_srl_epi32(__biased_value, __divisor_information.shift2()));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<int16>(__right);

			const auto __high_half_product = _mm_mulhi_epi16(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());
			const auto __sum_with_left_value = _mm_add_epi16(__high_half_product, __intrin_bitcast<__m128i>(__left));

			const auto __shifted_sum = _mm_sra_epi16(__sum_with_left_value, __divisor_information.shift());
			const auto __left_sign_mask = _mm_srai_epi16(__intrin_bitcast<__m128i>(__left), 15);
			const auto __sign_correction = _mm_sub_epi16(__left_sign_mask, __divisor_information.sign());
			const auto __adjusted_value = _mm_sub_epi16(__shifted_sum, __sign_correction);

			return __intrin_bitcast<_IntrinType_>(_mm_xor_si128(__adjusted_value, __divisor_information.sign()));
		}
		else if constexpr (__is_epu16_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<uint16>(__right);

			const auto __high_half_product = _mm_mulhi_epu16(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());
			const auto __difference_value = _mm_sub_epi16(__intrin_bitcast<__m128i>(__left), __high_half_product);

			const auto __first_shifted_value = _mm_srl_epi16(__difference_value, __divisor_information.shift1());
			const auto __biased_value = _mm_add_epi16(__high_half_product, __first_shifted_value);

			return __intrin_bitcast<_IntrinType_>(_mm_srl_epi16(__biased_value, __divisor_information.shift2()));
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			const auto __sign = _mm_cmpgt_epi8(_mm_setzero_si128(), __intrin_bitcast<__m128i>(__left));

			const auto __low = _Div<arch::ISA::SSE2, 128, int16>()(_mm_unpacklo_epi8(
				__intrin_bitcast<__m128i>(__left), __sign), __right);
			const auto __high = _Div<arch::ISA::SSE2, 128, int16>()(_mm_unpackhi_epi8(
				__intrin_bitcast<__m128i>(__left), __sign), __right);

			const auto __mask		= _mm_set1_epi32(0x00FF00FF);
			
			const auto __low_mask	= _mm_and_si128(__low, __mask);
			const auto __high_mask	= _mm_and_si128(__high, __mask);

			return __intrin_bitcast<_IntrinType_>(_mm_packus_epi16(__low_mask, __high_mask));
		}
		else if constexpr (__is_epu8_v<_DesiredType_>) {
			const auto __low_part = _mm_unpacklo_epi8(__intrin_bitcast<__m128i>(__left), _mm_setzero_si128());
			const auto __high_part = _mm_unpackhi_epi8(__intrin_bitcast<__m128i>(__left), _mm_setzero_si128());

			const auto __low = _Div<arch::ISA::SSE2, 128, uint16>()(__low_part, __right);
			const auto __high = _Div<arch::ISA::SSE2, 128, uint16>()(__high_part, __right);

			const auto __mask = _mm_set1_epi32(0x00FF00FF);

			const auto __low_mask = _mm_and_si128(__low, __mask);
			const auto __high_mask = _mm_and_si128(__high, __mask);

			return __intrin_bitcast<_IntrinType_>(_mm_packus_epi16(__low_mask, __high_mask));
		}
	}

	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right) raze_const_operator noexcept
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_div_pd(
				__intrin_bitcast<__m128d>(__left), __intrin_bitcast<__m128d>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_div_ps(
				__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__right)));
		}
		else if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			const auto __left_0 = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__left));
			const auto __right_0 = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__right));

			const auto __left_1 = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(_mm_movehl_ps(
				__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__left))));

			const auto __right_1 = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(_mm_movehl_ps(
				__intrin_bitcast<__m128>(__right), __intrin_bitcast<__m128>(__right))));

			return _mm_set_epi64x(__left_1 / __right_1, __left_0 / __right_0);
		}
		else {
			return __div_fallback<arch::ISA::SSE2, 128, _DesiredType_>(__left, __right);
		}
	}
};

template <class _DesiredType_> 
struct _Div<arch::ISA::AVX, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right) raze_const_operator noexcept
	{
		return (*this)(__left, _Broadcast<arch::ISA::AVX, 256, _IntrinType_>()(__right));
	}

	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_div_ps(
				__intrin_bitcast<__m256>(__left), 
				__intrin_bitcast<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_div_pd(
				__intrin_bitcast<__m256d>(__left),
				__intrin_bitcast<__m256d>(__right)));
		}
		else {
			const auto __low = _Div<arch::ISA::SSE42, 128, _DesiredType_>()(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));
			const auto __high = _Div<arch::ISA::SSE42, 128, _DesiredType_>()(
				_mm256_extractf128_si256(__intrin_bitcast<__m256i>(__left), 1),
				_mm256_extractf128_si256(__intrin_bitcast<__m256i>(__right), 1));

			return __intrin_bitcast<_IntrinType_>(_mm256_insertf128_si256(
				__intrin_bitcast<__m256i>(__low), __high, 1));
		}
	}
};

template <class _DesiredType_>
struct _Div<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			return __div_fallback<arch::ISA::AVX2, 256>(__left, __right);
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<int32>(__right);

			const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
			const auto __sign_broadcast = _mm256_broadcastq_epi64(__divisor_information.sign());

			const auto __low_half_product = _mm256_mul_epi32(__intrin_bitcast<__m256i>(__left), __multiplier_broadcast);
			const auto __low_half_product_shifted = _mm256_srli_epi64(__low_half_product, 32);

			const auto __left_value_shifted = _mm256_srli_epi64(__intrin_bitcast<__m256i>(__left), 32);
			const auto __high_half_product = _mm256_mul_epi32(__left_value_shifted, __multiplier_broadcast);

			const auto __merged_product = _mm256_blend_epi32(__low_half_product_shifted, __high_half_product, 0xAA);

			const auto __biased_value = _mm256_add_epi32(__merged_product, __intrin_bitcast<__m256i>(__left));
			const auto __shifted_value = _mm256_sra_epi32(__biased_value, __divisor_information.shift());

			const auto __left_sign_mask = _mm256_srai_epi32(__intrin_bitcast<__m256i>(__left), 31);

			const auto __sign_correction = _mm256_sub_epi32(__left_sign_mask, __sign_broadcast);
			const auto __adjusted_value = _mm256_sub_epi32(__shifted_value, __sign_correction);

			return __intrin_bitcast<_IntrinType_>(_mm256_xor_si256(__adjusted_value, __sign_broadcast));
		}
		else if constexpr (__is_epu32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<uint32>(__right);

			const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
			const auto __low_half_product = _mm256_mul_epu32(__intrin_bitcast<__m256i>(__left), __multiplier_broadcast);
			const auto __low_half_product_shifted = _mm256_srli_epi64(__low_half_product, 32);

			const auto __left_value_shifted = _mm256_srli_epi64(__intrin_bitcast<__m256i>(__left), 32);
			const auto __high_half_product = _mm256_mul_epu32(__left_value_shifted, __multiplier_broadcast);

			const auto __merged_product = _mm256_blend_epi32(__low_half_product_shifted, __high_half_product, 0xAA);
			const auto __difference_value = _mm256_sub_epi32(__intrin_bitcast<__m256i>(__left), __merged_product);

			const auto __first_shifted_value = _mm256_srl_epi32(__difference_value, __divisor_information.shift1());
			const auto __biased_value = _mm256_add_epi32(__merged_product, __first_shifted_value);

			return __intrin_bitcast<_IntrinType_>(_mm256_srl_epi32(__biased_value, __divisor_information.shift2()));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<int16>(__right);

			const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
			const auto __sign_broadcast = _mm256_broadcastq_epi64(__divisor_information.sign());

			const auto __high_half_product = _mm256_mulhi_epi16(__intrin_bitcast<__m256i>(__left), __multiplier_broadcast);
			const auto __sum_with_left_value = _mm256_add_epi16(__high_half_product, __intrin_bitcast<__m256i>(__left));

			const auto __shifted_sum = _mm256_sra_epi16(__sum_with_left_value, __divisor_information.shift());
			const auto __left_sign_mask = _mm256_srai_epi16(__intrin_bitcast<__m256i>(__left), 15);

			const auto __sign_correction = _mm256_sub_epi16(__left_sign_mask, __sign_broadcast);
			const auto __adjusted_value = _mm256_sub_epi16(__shifted_sum, __sign_correction);

			return __intrin_bitcast<_IntrinType_>(_mm256_xor_si256(__adjusted_value, __sign_broadcast));
		}
		else if constexpr (__is_epu16_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<uint16>(__right);

			const auto __multiplier_broadcast = _mm256_broadcastq_epi64(__divisor_information.multiplier());
			const auto __high_half_product = _mm256_mulhi_epu16(__intrin_bitcast<__m256i>(__left), __multiplier_broadcast);

			const auto __difference_value = _mm256_sub_epi16(__intrin_bitcast<__m256i>(__left), __high_half_product);
			const auto __first_shifted_value = _mm256_srl_epi16(__difference_value, __divisor_information.shift1());

			const auto __biased_value = _mm256_add_epi16(__high_half_product, __first_shifted_value);
			return __intrin_bitcast<_IntrinType_>(_mm256_srl_epi16(__biased_value, __divisor_information.shift2()));
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			const auto __zeros = _mm256_setzero_si256();
			const auto __and_mask = _mm256_set1_epi32(0x00FF00FF);

			const auto __shuffled_low = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__left), 0x10);
			const auto __shuffled_high = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__left), 0xC8);

			const auto __sign_low = _mm256_cmpgt_epi8(__zeros, __shuffled_low);
			const auto __sign_high = _mm256_cmpgt_epi8(__zeros, __shuffled_high);

			const auto __low = _mm256_unpacklo_epi8(__shuffled_low, __sign_low);
			const auto __high = _mm256_unpackhi_epi8(__shuffled_high, __sign_high);

			const auto __divided_low = _Div<arch::ISA::AVX2, 256, int16>()(__low, __right);
			const auto __divided_high = _Div<arch::ISA::AVX2, 256, int16>()(__high, __right);

			const auto __low_mask = _mm256_and_si256(__divided_low, __and_mask);
			const auto __high_mask = _mm256_and_si256(__divided_high, __and_mask);

			return __intrin_bitcast<_IntrinType_>(_mm256_permute4x64_epi64(
				_mm256_packus_epi16(__low_mask, __high_mask), 0xD8));
		}
		else if constexpr (__is_epu8_v<_DesiredType_>) {
			const auto __zeros = _mm256_setzero_si256();
			const auto __and_mask = _mm256_set1_epi32(0x00FF00FF);

			const auto __shuffled_low = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__left), 0x10);
			const auto __shuffled_high = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__left), 0xC8);

			const auto __low = _mm256_unpacklo_epi8(__shuffled_low, __zeros);
			const auto __high = _mm256_unpackhi_epi8(__shuffled_high, __zeros);

			const auto __divided_low = _Div<arch::ISA::AVX2, 256, int16>()(__low, __right);
			const auto __divided_high = _Div<arch::ISA::AVX2, 256, int16>()(__high, __right);

			const auto __low_mask = _mm256_and_si256(__divided_low, __and_mask);
			const auto __high_mask = _mm256_and_si256(__divided_high, __and_mask);

			return __intrin_bitcast<_IntrinType_>(_mm256_permute4x64_epi64(
				_mm256_packus_epi16(__low_mask, __high_mask), 0xD8));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_div_ps(
				__intrin_bitcast<__m256>(__left), _mm256_set1_ps(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_div_pd(
				__intrin_bitcast<__m256d>(__left), _mm256_set1_pd(__right)));
		}
	}

	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			const auto __left_high128 = _mm256_extracti128_si256(__intrin_bitcast<__m256i>(__left), 1);
			const auto __right_high128 = _mm256_extracti128_si256(__intrin_bitcast<__m256i>(__right), 1);

			const auto __left_low_q = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__left_high128));
			const auto __right_low_q = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__right_high128));

			const auto __left_low2_q = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__left));
			const auto __right_low2_q = _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__right));

			const auto __left_high2_q = _mm_extract_epi64(__intrin_bitcast<__m128i>(__left), 1);
			const auto __right_high2_q = _mm_extract_epi64(__intrin_bitcast<__m128i>(__right), 1);

			const auto __left_high_q = _mm_extract_epi64(__left_high128, 1);
			const auto __right_high_q = _mm_extract_epi64(__right_high128, 1);

			const auto __high_high_dividend = __left_high_q;
			const auto __high_high_divisor = __right_high_q;
			const auto __high_high_result = __high_high_dividend / __high_high_divisor;

			const auto __low_high_dividend = __left_low_q;
			const auto __low_high_divisor = __right_low_q;
			const auto __low_high_result = __low_high_dividend / __low_high_divisor;

			const auto __low_low_result = __left_low2_q / __right_low2_q;
			const auto __high_low_result = __left_high2_q / __right_high2_q;

			const auto __pack_low = _mm_unpacklo_epi64(_mm_cvtsi64_si128(__low_low_result), _mm_cvtsi64_si128(__high_low_result));
			const auto __pack_high = _mm_unpacklo_epi64(_mm_cvtsi64_si128(__low_high_result), _mm_cvtsi64_si128(__high_high_result));

			return __intrin_bitcast<_IntrinType_>(_mm256_inserti128_si256(_mm256_castsi128_si256(__pack_low), __pack_high, 1));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_div_ps(
				__intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_div_pd(
				__intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right)));
		}
		else {
			return __div_fallback<arch::ISA::AVX2, 256, _DesiredType_>(__left, __right);
		}
	}
};

template <class _DesiredType_>
struct _Div<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right) raze_const_operator noexcept
	{
		if constexpr (__is_epi32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<int32>(__right);

			const auto __multiplier_broadcast = _mm512_broadcast_i32x4(__divisor_information.multiplier());
			const auto __sign_broadcast = _mm512_broadcast_i32x4(__divisor_information.sign());

			const auto __low_half_product = _mm512_mul_epi32(__intrin_bitcast<__m512i>(__left), __multiplier_broadcast);
			const auto __left_shifted_high = _mm512_srli_epi64(__intrin_bitcast<__m512i>(__left), 32);

			const auto __high_half_product = _mm512_mul_epi32(__left_shifted_high, __multiplier_broadcast);
			const auto __low_half_product_shifted = _mm512_srli_epi64(__low_half_product, 32);

			const auto __merged_product = _mm512_mask_mov_epi32(__low_half_product_shifted, 0xAAAA, __high_half_product);
			const auto __biased_value = _mm512_add_epi32(__merged_product, __intrin_bitcast<__m512i>(__left));

			const auto __shifted_value = _mm512_sra_epi32(__biased_value, __divisor_information.shift());
			const auto __left_sign_mask = _mm512_srai_epi32(__intrin_bitcast<__m512i>(__left), 31);

			const auto __sign_correction = _mm512_sub_epi32(__left_sign_mask, __sign_broadcast);
			const auto __adjusted_value = _mm512_sub_epi32(__shifted_value, __sign_correction);

			return __intrin_bitcast<_IntrinType_>(_mm512_xor_si512(__adjusted_value, __sign_broadcast));
		}
		else if constexpr (__is_epu32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<uint32>(__right);

			const auto __multiplier_broadcast = _mm512_broadcast_i32x4(__divisor_information.multiplier());
			const auto __low_half_product = _mm512_mul_epu32(__intrin_bitcast<__m512i>(__left), __multiplier_broadcast);

			const auto __left_shifted_high = _mm512_srli_epi64(__intrin_bitcast<__m512i>(__left), 32);
			const auto __high_half_product = _mm512_mul_epu32(__left_shifted_high, __multiplier_broadcast);

			const auto __low_half_product_shifted = _mm512_srli_epi64(__low_half_product, 32);
			const auto __merged_product = _mm512_mask_mov_epi32(__low_half_product_shifted, 0xAAAA, __high_half_product);
			const auto __difference_value = _mm512_sub_epi32(__intrin_bitcast<__m512i>(__left), __merged_product);

			const auto __first_shifted_value = _mm512_srl_epi32(__difference_value, __divisor_information.shift1());
			const auto __biased_value = _mm512_add_epi32(__merged_product, __first_shifted_value);

			return _mm512_srl_epi32(__biased_value, __divisor_information.shift2());
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_div_pd(
				__intrin_bitcast<__m512d>(__left), _mm512_set1_pd(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_div_ps(
				__intrin_bitcast<__m512>(__left), _mm512_set1_ps(__right)));
		}
		else {
			const auto __low = _Div<arch::ISA::AVX2, 256, _DesiredType_>()(
				__intrin_bitcast<__m256i>(__left), __right);
			const auto __high = _Div<arch::ISA::AVX2, 256, _DesiredType_>()(
				_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__left), 1), __right);

			return __intrin_bitcast<_IntrinType_>(_mm512_insertf64x4(
				__intrin_bitcast<__m512d>(__low), __intrin_bitcast<__m256d>(__high), 1));
		}
	}

	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right) raze_const_operator noexcept
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_div_pd(
				__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_div_ps(
				__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right)));
		}
		else {
			return __div_fallback<arch::ISA::AVX512F, 512, _DesiredType_>(__left, __right);
		}
	}
};

template <class _DesiredType_> struct _Div<arch::ISA::SSE3, 128, _DesiredType_> : _Div<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::SSSE3, 128, _DesiredType_> : _Div<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::SSE41, 128, _DesiredType_> : _Div<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::SSE42, 128, _DesiredType_> : _Div<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX, 128, _DesiredType_> : _Div<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::FMA3, 128, _DesiredType_> : _Div<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX2, 128, _DesiredType_> : _Div<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _Div<arch::ISA::AVX2, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Div<arch::ISA::AVX512BW, 512, _DesiredType_> : _Div<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Div<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Div<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Div<arch::ISA::FMA3, 256, _DesiredType_> : _Div<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX2FMA3, 256, _DesiredType_> : _Div<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Div<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Div<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Div<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Div<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Div<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Div<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Div<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Div<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Div<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Div<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Div<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Div<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Div<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Div<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Div<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Div<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
