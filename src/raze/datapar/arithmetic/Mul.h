#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/Blend.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Mul;

template <class _DesiredType_>
struct _Mul<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mul_pd(
				__intrin_bitcast<__m128d>(__left), __intrin_bitcast<__m128d>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mul_ps(
				__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__right)));
		}
		else if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			alignas(16) int64 __left_array[2];
			alignas(16) int64 __right_array[2];
			
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&__left_array), __intrin_bitcast<__m128i>(__left));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&__right_array), __intrin_bitcast<__m128i>(__right));

			return _mm_set_epi64x(__left_array[1] * __right_array[1], __left_array[0] * __right_array[0]);
		}
		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
			const auto __shuffled_left = _mm_shuffle_epi32(__intrin_bitcast<__m128i>(__left), 0xF5);
			const auto __shuffled_right = _mm_shuffle_epi32(__intrin_bitcast<__m128i>(__right), 0xF5);

			const auto __product_02 = _mm_mul_epu32(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));
			const auto __product_13 = _mm_mul_epu32(__shuffled_left, __shuffled_right);

			const auto __product_01 = _mm_unpacklo_epi32(__product_02, __product_13);
			const auto __product_23 = _mm_unpackhi_epi32(__product_02, __product_13);

			return __intrin_bitcast<_IntrinType_>(_mm_unpacklo_epi64(__product_01, __product_23));
		}
		else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mullo_epi16(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
			const auto __left_odd = _mm_srli_epi16(__intrin_bitcast<__m128i>(__left), 8);
			const auto __right_odd = _mm_srli_epi16(__intrin_bitcast<__m128i>(__right), 8);

			const auto __multiplied_even = _mm_mullo_epi16(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));
			const auto __multiplied_odd = _mm_slli_epi16(_mm_mullo_epi16(__left_odd, __right_odd), 8);

			return __intrin_bitcast<_IntrinType_>(_Blend<arch::ISA::SSE2, 128, _DesiredType_>()(
				__multiplied_even, __multiplied_odd, _mm_set1_epi32(0x00FF00FF)));
		}
	}
};

template <class _DesiredType_> 
struct _Mul<arch::ISA::AVX, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mul_ps(
				__intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mul_pd(
				__intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right)));
		}
		else {
			const auto __low = _Mul<arch::ISA::SSE42, 128, _DesiredType_>()(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));
			const auto __high = _Mul<arch::ISA::SSE42, 128, _DesiredType_>()(
				_mm256_extractf128_si256(__intrin_bitcast<__m256i>(__left), 1),
				_mm256_extractf128_si256(__intrin_bitcast<__m256i>(__right), 1));

			return __intrin_bitcast<_IntrinType_>(_mm256_insertf128_si256(
				__intrin_bitcast<__m256i>(__low), __high, 1));
		}
	}
};

template <class _DesiredType_>
struct _Mul<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
			const auto __left_odd = _mm256_srli_epi16(__intrin_bitcast<__m256i>(__left), 8);
			const auto __right_odd = _mm256_srli_epi16(__intrin_bitcast<__m256i>(__right), 8);

			const auto __multiplied_even = _mm256_mullo_epi16(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
			const auto __multiplied_odd = _mm256_slli_epi16(_mm256_mullo_epi16(__left_odd, __right_odd), 8);

			return __intrin_bitcast<_IntrinType_>(_Blend<arch::ISA::AVX2, 256, _DesiredType_>()(
				__multiplied_even, __multiplied_odd, _mm256_set1_epi32(0x00FF00FF)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mullo_epi16(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mullo_epi32(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			const auto __swapped = _mm256_shuffle_epi32(__intrin_bitcast<__m256i>(__right), 0xB1);
			const auto __multiplied_32 = _mm256_mullo_epi32(__intrin_bitcast<__m256i>(__left), __swapped);

			const auto __add_32 = _mm256_hadd_epi32(__multiplied_32, _mm256_setzero_si256());
			const auto __shuffled_32 = _mm256_shuffle_epi32(__add_32, 0x73);

			const auto __final_32 = _mm256_mul_epu32(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
			return __intrin_bitcast<_IntrinType_>(_mm256_add_epi64(__final_32, __shuffled_32));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mul_ps(
				__intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mul_pd(
				__intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right)));
		}
	}
};

template <class _DesiredType_>
struct _Mul<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
			const auto __left_high = _mm512_srli_epi64(__intrin_bitcast<__m512i>(__left), 32);
			const auto __right_high = _mm512_srli_epi64(__intrin_bitcast<__m512i>(__right), 32);

			const auto __multiplied1 = _mm512_mul_epu32(__left_high, __intrin_bitcast<__m512i>(__right));
			const auto __multiplied2 = _mm512_mul_epu32(__right_high, __intrin_bitcast<__m512i>(__left));

			const auto __add64 = _mm512_add_epi64(__multiplied2, __multiplied1);

			const auto __high = _mm512_slli_epi64(__add64, 32);
			const auto __low = _mm512_mul_epu32(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));

			return __intrin_bitcast<_IntrinType_>(_mm512_add_epi64(__low, __high));
		}
		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mullo_epi32(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mul_ps(
				__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mul_pd(
				__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right)));
		}
		else {
			const auto __low = _Mul<arch::ISA::AVX2, 256, _DesiredType_>()(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));

			const auto __high = _Mul<arch::ISA::AVX2, 256, _DesiredType_>()(
				_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__left), 1),
				_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__right), 1));

			return __intrin_bitcast<_IntrinType_>(_mm512_inserti64x4(
				__intrin_bitcast<__m512i>(__low), __intrin_bitcast<__m256i>(__high), 1));
		}
	}
};

template <class _DesiredType_> struct _Mul<arch::ISA::SSE3, 128, _DesiredType_> : _Mul<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::SSSE3, 128, _DesiredType_> : _Mul<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::SSE41, 128, _DesiredType_> : _Mul<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::SSE42, 128, _DesiredType_> : _Mul<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX, 128, _DesiredType_> : _Mul<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX2, 128, _DesiredType_> : _Mul<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::FMA3, 128, _DesiredType_> : _Mul<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _Mul<arch::ISA::AVX2, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Mul<arch::ISA::AVX512BW, 512, _DesiredType_> : _Mul<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Mul<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Mul<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Mul<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Mul<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Mul<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Mul<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Mul<arch::ISA::FMA3, 256, _DesiredType_> : _Mul<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX2FMA3, 256, _DesiredType_> : _Mul<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Mul<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Mul<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Mul<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mul<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Mul<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
