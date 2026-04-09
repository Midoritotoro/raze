#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Not;

template <>
struct _Not<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m128d>)
			return _mm_xor_pd(__vector, __intrin_bitcast<__m128d>(_mm_set1_epi32(-1)));

		else if constexpr (std::is_same_v<_IntrinType_, __m128i>)
			return _mm_xor_si128(__vector, _mm_set1_epi32(-1));

		else if constexpr (std::is_same_v<_IntrinType_, __m128>)
			return _mm_xor_ps(__vector, __intrin_bitcast<__m128>(_mm_set1_epi32(-1)));
	}
};


template <> struct _Not<arch::ISA::SSE3, 128> : _Not<arch::ISA::SSE2, 128> {};
template <> struct _Not<arch::ISA::SSSE3, 128> : _Not<arch::ISA::SSE3, 128> {};
template <> struct _Not<arch::ISA::SSE41, 128> : _Not<arch::ISA::SSSE3, 128> {};
template <> struct _Not<arch::ISA::SSE42, 128> : _Not<arch::ISA::SSE41, 128> {};
template <> struct _Not<arch::ISA::AVX, 128> : _Not<arch::ISA::SSE42, 128> {};
template <> struct _Not<arch::ISA::FMA3, 128> : _Not<arch::ISA::AVX, 128> {};
template <> struct _Not<arch::ISA::AVX2, 128> : _Not<arch::ISA::AVX, 128> {};
template <> struct _Not<arch::ISA::AVX2FMA3, 128> : _Not<arch::ISA::AVX2, 128> {};


template <> 
struct _Not<arch::ISA::AVX512VLF, 128>:
	_Not<arch::ISA::AVX2, 128> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm_ternarylogic_epi32(
			__intrin_bitcast<__m128i>(__vector), __intrin_bitcast<__m128i>(__vector),
			__intrin_bitcast<__m128i>(__vector), 0x55));
	}
};

template <> struct _Not<arch::ISA::AVX, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		if constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_xor_pd(__vector, __intrin_bitcast<__m256d>(_mm256_set1_epi32(-1)));

		else
			return __intrin_bitcast<_IntrinType_>(_mm256_xor_ps(
				__intrin_bitcast<__m256>(__vector),
				__intrin_bitcast<__m256>(_mm256_set1_epi32(-1))));
	}
};

template <>
struct _Not<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_xor_pd(__vector, __intrin_bitcast<__m256d>(_mm256_set1_epi32(-1)));

		else if constexpr (std::is_same_v<_IntrinType_, __m256i>)
			return _mm256_xor_si256(__vector, _mm256_set1_epi32(-1));

		else if constexpr (std::is_same_v<_IntrinType_, __m256>)
			return _mm256_xor_ps(__vector, __intrin_bitcast<__m256>(_mm256_set1_epi32(-1)));
	}
};

template <> struct _Not<arch::ISA::FMA3, 256> : _Not<arch::ISA::AVX, 256> {};
template <> struct _Not<arch::ISA::AVX2FMA3, 256> : _Not<arch::ISA::AVX2, 256> {};

template <> 
struct _Not<arch::ISA::AVX512VLF, 256>: 
	_Not<arch::ISA::AVX2, 256> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm256_ternarylogic_epi32(
			__intrin_bitcast<__m256i>(__vector), __intrin_bitcast<__m256i>(__vector),
			__intrin_bitcast<__m256i>(__vector), 0x55));
	}
};


template <>
struct _Not<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		return __intrin_bitcast<_IntrinType_>(_mm512_ternarylogic_epi32(
			__intrin_bitcast<__m512i>(__vector), __intrin_bitcast<__m512i>(__vector),
			__intrin_bitcast<__m512i>(__vector), 0x55));
	}
};

template <> struct _Not<arch::ISA::AVX512BW, 512> : _Not<arch::ISA::AVX512F, 512> {};
template <> struct _Not<arch::ISA::AVX512DQ, 512> : _Not<arch::ISA::AVX512F, 512> {};
template <> struct _Not<arch::ISA::AVX512BWDQ, 512> : _Not<arch::ISA::AVX512BW, 512> {};
template <> struct _Not<arch::ISA::AVX512VBMI, 512> : _Not<arch::ISA::AVX512BW, 512> {};
template <> struct _Not<arch::ISA::AVX512VBMI2, 512> : _Not<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Not<arch::ISA::AVX512VBMIDQ, 512> : _Not<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Not<arch::ISA::AVX512VBMI2DQ, 512> : _Not<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Not<arch::ISA::AVX512VLBW, 256> : _Not<arch::ISA::AVX512VLF, 256> {};
template <> struct _Not<arch::ISA::AVX512VLDQ, 256> : _Not<arch::ISA::AVX512VLF, 256> {};
template <> struct _Not<arch::ISA::AVX512VLBWDQ, 256> : _Not<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Not<arch::ISA::AVX512VBMIVL, 256> : _Not<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Not<arch::ISA::AVX512VBMI2VL, 256> : _Not<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Not<arch::ISA::AVX512VBMIVLDQ, 256> : _Not<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Not<arch::ISA::AVX512VBMI2VLDQ, 256> : _Not<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Not<arch::ISA::AVX512VLBW, 128> : _Not<arch::ISA::AVX512VLF, 128> {};
template <> struct _Not<arch::ISA::AVX512VLDQ, 128> : _Not<arch::ISA::AVX512VLF, 128> {};
template <> struct _Not<arch::ISA::AVX512VLBWDQ, 128> : _Not<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Not<arch::ISA::AVX512VBMIVL, 128> : _Not<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Not<arch::ISA::AVX512VBMI2VL, 128> : _Not<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Not<arch::ISA::AVX512VBMIVLDQ, 128> : _Not<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Not<arch::ISA::AVX512VBMI2VLDQ, 128> : _Not<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
