#pragma once 

#include <src/raze/datapar/compare/Greater.h>
#include <src/raze/datapar/shuffle/Blend.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_vertical_max;

template <class _DesiredType_>
struct _Simd_vertical_max<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_epi16(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else if constexpr (__is_epu8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_epu8(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_ps(__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_pd(__intrin_bitcast<__m128d>(__left), __intrin_bitcast<__m128d>(__right)));
		}
		else {
			const auto __mask = _Simd_greater<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right);
			return _Simd_blend<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right, __mask);
		}
	}
};

template <class _DesiredType_> 
struct _Simd_vertical_max<arch::ISA::SSE41, 128, _DesiredType_>:
	_Simd_vertical_max<arch::ISA::SSSE3, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_epi32(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else if constexpr (__is_epu32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_epu32(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else if constexpr (__is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_epu16(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_max_epi8(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
		}
		else {
			return _Simd_vertical_max<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right);
		}
	}
};

template <class _DesiredType_>
struct _Simd_vertical_max<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epi32(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epu32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epu32(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epi16(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epu16(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epi8(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_epu8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epu8(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_ps(
				__intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_max_pd(
				__intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right)));
		}
		else {
			const auto __mask = _Simd_greater<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right);
			return _Simd_blend<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Simd_vertical_max<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epi64(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));
		}
		else if constexpr (__is_epu64_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epu64(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epi32(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));
		}
		else if constexpr (__is_epu32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epu32(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_max_ps(
				__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_max_pd(
				__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right)));
		}
		else {
			const auto __mask = _Simd_greater<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right);
			return _Simd_blend<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Simd_vertical_max<arch::ISA::AVX512BW, 512, _DesiredType_>:
	_Simd_vertical_max<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi8_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epi8(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));

		else if constexpr (__is_epu8_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epu8(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));

		else if constexpr (__is_epi16_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epi16(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));

		else if constexpr (__is_epu16_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_max_epu16(
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));

		else
			return _Simd_vertical_max<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right);
	}
};

template <class _DesiredType_> 
struct _Simd_vertical_max<arch::ISA::AVX512VLF, 256, _DesiredType_>: 
	_Simd_vertical_max<arch::ISA::AVX2, 256, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epi64(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));

		else if constexpr (__is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_max_epu64(
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));

		else
			return _Simd_vertical_max<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right);
	}
};

template <class _DesiredType_> 
struct _Simd_vertical_max<arch::ISA::AVX512VLF, 128, _DesiredType_>: 
	_Simd_vertical_max<arch::ISA::SSE42, 128, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_max_epi64(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));

		else if constexpr (__is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_max_epu64(
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));

		else
			return _Simd_vertical_max<arch::ISA::SSE42, 128, _DesiredType_>()(__left, __right);
	}
};

template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_vertical_max<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_vertical_max<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
