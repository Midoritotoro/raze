

#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_abs;

template <class _DesiredType_>
struct _Simd_abs<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return __vector;
		}
		else if constexpr (__is_epi64_v<_DesiredType_>) {
			const auto __high_sign = _mm_srai_epi32(__intrin_bitcast<__m128i>(__vector), 31);
			const auto __sign = _mm_shuffle_epi32(__high_sign, 0xF5);

			const auto __invert = _mm_xor_si128(__intrin_bitcast<__m128i>(__vector), __sign);
			return __intrin_bitcast<_IntrinType_>(_mm_sub_epi64(__invert, __sign));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			const auto __sign = _mm_srai_epi32(__intrin_bitcast<__m128i>(__vector), 31);
			const auto __invert = _mm_xor_si128(__intrin_bitcast<__m128i>(__vector), __sign);

			return __intrin_bitcast<_IntrinType_>(_mm_sub_epi32(__invert, __sign));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			const auto __negate = _mm_sub_epi16(_mm_setzero_si128(), __intrin_bitcast<__m128i>(__vector));
			return _mm_max_epi16(__intrin_bitcast<__m128i>(__vector), __negate);
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			const auto __negate = _mm_sub_epi8(_mm_setzero_si128(), __intrin_bitcast<__m128i>(__vector));
			return __intrin_bitcast<_IntrinType_>(_mm_min_epu8(__intrin_bitcast<__m128i>(__vector), __negate));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			const auto __mask = _mm_set1_epi32(0x7FFFFFFF);
			return __intrin_bitcast<_IntrinType_>(_mm_and_ps(__intrin_bitcast<__m128>(__vector), __intrin_bitcast<__m128>(__mask)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			const auto __mask = _mm_set_epi32(0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu);
			return __intrin_bitcast<_IntrinType_>(_mm_and_pd(__intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__mask)));
		}
	}
};

template <class _DesiredType_>
struct _Simd_abs<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return __vector;
		}
		else if constexpr (__is_epi64_v<_DesiredType_>) {
			const auto __sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), __vector);
			const auto __inverted = _mm256_xor_si256(__intrin_bitcast<__m256i>(__vector), __sign);

			return __intrin_bitcast<_IntrinType_>(_mm256_sub_epi64(__inverted, __sign));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_abs_epi32(__intrin_bitcast<__m256i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_abs_epi16(__intrin_bitcast<__m256i>(__vector)));
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_abs_epi8(__intrin_bitcast<__m256i>(__vector)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			const auto __mask = _mm256_set_epi32(0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu,
				0xFFFFFFFFu, 0x7FFFFFFFu, 0xFFFFFFFFu, 0x7FFFFFFFu);
			return __intrin_bitcast<_IntrinType_>(_mm256_and_pd(
				__intrin_bitcast<__m256d>(__vector), __intrin_bitcast<__m256d>(__mask)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			const auto __mask = _mm256_set1_epi32(0x7FFFFFFFu);
			return __intrin_bitcast<_IntrinType_>(_mm256_and_ps(
				__intrin_bitcast<__m256>(__vector), __intrin_bitcast<__m256>(__mask)));
		}
	}
};

template <class _DesiredType_>
struct _Simd_abs<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return __vector;
		}
		else if constexpr (__is_epi64_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_abs_epi64(__intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_abs_epi32(__intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_abs_ps(__intrin_bitcast<__m512>(__vector)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_abs_pd(__intrin_bitcast<__m512d>(__vector)));
		}
		else {
			const auto __low = __intrin_bitcast<__m256i>(__vector);
			const auto __high = _mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1);

			auto __result = __intrin_bitcast<__m512i>(_Simd_abs<arch::ISA::AVX2, 256, _DesiredType_>()(__low));
			__result = __intrin_bitcast<__m512i>(_mm512_insertf64x4(__intrin_bitcast<__m512d>(__result),
				__intrin_bitcast<__m256d>(_Simd_abs<arch::ISA::AVX2, 256, _DesiredType_>()(__high)), 1));

			return __intrin_bitcast<_IntrinType_>(__result);
		}
	}
};

template <class _DesiredType_>
struct _Simd_abs<arch::ISA::AVX512BW, 512, _DesiredType_>:
	_Simd_abs<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (__is_epi16_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_abs_epi16(__intrin_bitcast<__m512i>(__vector)));

		else if constexpr (__is_epi8_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_abs_epi8(__intrin_bitcast<__m512i>(__vector)));

		else
			return _Simd_abs<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_> 
struct _Simd_abs<arch::ISA::AVX512VLF, 256, _DesiredType_>:
	_Simd_abs<arch::ISA::AVX2, 256, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_abs_epi64(__intrin_bitcast<__m256i>(__vector)));
		else
			return _Simd_abs<arch::ISA::AVX2, 256, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_>
struct _Simd_abs<arch::ISA::AVX512VLF, 128, _DesiredType_> :
	_Simd_abs<arch::ISA::AVX2, 128, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_abs_epi64(__intrin_bitcast<__m128i>(__vector)));
		else
			return _Simd_abs<arch::ISA::SSE42, 128, _DesiredType_>()(__vector);
	}
};

template <class _DesiredType_> struct _Simd_abs<arch::ISA::SSE3, 128, _DesiredType_>: _Simd_abs<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::SSSE3, 128, _DesiredType_>: _Simd_abs<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::SSE41, 128, _DesiredType_>: _Simd_abs<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::SSE42, 128, _DesiredType_>: _Simd_abs<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX2, 128, _DesiredType_>: _Simd_abs<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512DQ, 512, _DesiredType_>: _Simd_abs<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512BWDQ, 512, _DesiredType_>: _Simd_abs<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI, 512, _DesiredType_>: _Simd_abs<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI2, 512, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_>: _Simd_abs<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VLBW, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VLDQ, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMIVL, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VLBW, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VLDQ, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMIVL, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_abs<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_>: _Simd_abs<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
