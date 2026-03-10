#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/Blend.h>
#include <src/raze/datapar/arithmetic/Divisor.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Div;

template <class _DesiredType_>
struct _Div<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
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
				__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__left))));

			return _mm_set_epi64x(__left_1 / __right_1, __left_0 / __right_0);
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<arch::ISA::SSE2, int32>(
				_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__right)));

			const auto __low_product = _mm_mul_epu32(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());

			const auto __low_product_shifted = _mm_srli_epi64(__low_product, 32);
			const auto __left_value_shifted = _mm_srli_epi64(__intrin_bitcast<__m128i>(__left), 32);

			const auto __high_product = _mm_mul_epu32(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());

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
			const auto __divisor_information = _Divisor<arch::ISA::SSE2, uint32>(
				_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__right)));

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
			const auto __divisor_information = _Divisor<arch::ISA::SSE2, int16>(
				_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__right)));

			const auto __high_half_product = _mm_mulhi_epi16(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());
			const auto __sum_with_left_value = _mm_add_epi16(__high_half_product, __intrin_bitcast<__m128i>(__left));

			const auto __shifted_sum = _mm_sra_epi16(__sum_with_left_value, __divisor_information.shift());
			const auto __left_sign_mask = _mm_srai_epi16(__intrin_bitcast<__m128i>(__left), 15);
			const auto __sign_correction = _mm_sub_epi16(__left_sign_mask, __divisor_information.sign());
			const auto __adjusted_value = _mm_sub_epi16(__shifted_sum, __sign_correction);

			return __intrin_bitcast<_IntrinType_>(_mm_xor_si128(__adjusted_value, __divisor_information.sign()));
		}
		else if constexpr (__is_epu16_v<_DesiredType_>) {
			const auto __divisor_information = _Divisor<arch::ISA::SSE2, uint16>(
				_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__right)));

			const auto __high_half_product = _mm_mulhi_epu16(__intrin_bitcast<__m128i>(__left), __divisor_information.multiplier());
			const auto __difference_value = _mm_sub_epi16(__intrin_bitcast<__m128i>(__left), __high_half_product);

			const auto __first_shifted_value = _mm_srl_epi16(__difference_value, __divisor_information.shift1());
			const auto __biased_value = _mm_add_epi16(__high_half_product, __first_shifted_value);

			return __intrin_bitcast<_IntrinType_>(_mm_srl_epi16(__biased_value,__divisor_information.shift2()));
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			const auto __sign = _mm_cmpgt_epi8(_mm_setzero_si128(), __intrin_bitcast<__m128i>(__left));

			const auto __low = _Div<arch::ISA::SSE2, 128, int16>()(_mm_unpacklo_epi8(
				__intrin_bitcast<__m128i>(__left), __sign), __intrin_bitcast<__m128i>(__right));
			const auto __high = _Div<arch::ISA::SSE2, 128, int16>()(
				_mm_unpackhi_epi8(__intrin_bitcast<__m128i>(__left), __sign), __intrin_bitcast<__m128i>(__right));

			const auto __mask		= _mm_set1_epi32(0x00FF00FF);
			
			const auto __low_mask	= _mm_and_si128(__low, __mask);
			const auto __high_mask	= _mm_and_si128(__high, __mask);

			return __intrin_bitcast<_IntrinType_>(_mm_packus_epi16(__low_mask, __high_mask));
		}
		else if constexpr (__is_epu8_v<_DesiredType_>) {
			const auto __low = _Div<arch::ISA::SSE2, 128, uint16>()(_mm_unpacklo_epi8(
				__intrin_bitcast<__m128i>(__left), _mm_setzero_si128()), __intrin_bitcast<__m128i>(__right));
			const auto __high = _Div<arch::ISA::SSE2, 128, uint16>()(_mm_unpackhi_epi8(
				__intrin_bitcast<__m128i>(__left), _mm_setzero_si128()), __intrin_bitcast<__m128i>(__right));

			const auto __mask = _mm_set1_epi32(0x00FF00FF);

			const auto __low_mask = _mm_and_si128(__low, __mask);
			const auto __high_mask = _mm_and_si128(__high, __mask);

			return __intrin_bitcast<_IntrinType_>(_mm_packus_epi16(__low_mask, __high_mask));
		}
	}
};

template <class _DesiredType_>
struct _Div<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{

	}
};

template <class _DesiredType_>
struct _Div<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{

	}
};

template <class _DesiredType_> struct _Div<arch::ISA::SSE3, 128, _DesiredType_> : _Div<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::SSSE3, 128, _DesiredType_> : _Div<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::SSE41, 128, _DesiredType_> : _Div<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::SSE42, 128, _DesiredType_> : _Div<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX2, 128, _DesiredType_> : _Div<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Div<arch::ISA::AVX512BW, 512, _DesiredType_> : _Div<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Div<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Div<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Div<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Div<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

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
