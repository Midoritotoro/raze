#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>

#include <src/raze/datapar/compare/Equal.h>
#include <src/raze/datapar/bitwise/ToIndexMask.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Simd_testz;

template <>
struct _Simd_testz<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		const auto __zeros = _Simd_broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()();
		const auto __compared = _Simd_equal<arch::ISA::SSE2, 128, int32>()(__vector, __zeros);

		const auto __index_mask = _Simd_to_index_mask<arch::ISA::SSE2, 128, int8>()(__compared);
		return __index_mask == math::__maximum_integral_limit<decltype(__index_mask)>();
	}
};

template <> struct _Simd_testz<arch::ISA::SSE3, 128> : _Simd_testz<arch::ISA::SSE2, 128> {};
template <> struct _Simd_testz<arch::ISA::SSSE3, 128> : _Simd_testz<arch::ISA::SSE3, 128> {};

template <>
struct _Simd_testz<arch::ISA::SSE41, 128>: 
	_Simd_testz<arch::ISA::SSSE3, 128> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _mm_testz_si128(__intrin_bitcast<__m128i>(__vector), __intrin_bitcast<__m128i>(__vector));
	}
};


template <>
struct _Simd_testz<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		return _mm256_testz_si256(__intrin_bitcast<__m256i>(__vector), __intrin_bitcast<__m256i>(__vector));
	}
};

template <>
struct _Simd_testz<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		const auto __k = _mm512_test_epi64_mask(__intrin_bitcast<__m512i>(__vector), __intrin_bitcast<__m512i>(__vector));
		return _kortestz_mask16_u8(__k, __k);
	}
};

template <> struct _Simd_testz<arch::ISA::SSE42, 128> : _Simd_testz<arch::ISA::SSE41, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX2, 128> : _Simd_testz<arch::ISA::SSE42, 128> {};

template <> struct _Simd_testz<arch::ISA::AVX512BW, 512> : _Simd_testz<arch::ISA::AVX512F, 512> {};
template <> struct _Simd_testz<arch::ISA::AVX512DQ, 512> : _Simd_testz<arch::ISA::AVX512F, 512> {};
template <> struct _Simd_testz<arch::ISA::AVX512BWDQ, 512> : _Simd_testz<arch::ISA::AVX512BW, 512> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI, 512> : _Simd_testz<arch::ISA::AVX512BW, 512> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI2, 512> : _Simd_testz<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMIDQ, 512> : _Simd_testz<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI2DQ, 512> : _Simd_testz<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Simd_testz<arch::ISA::AVX512VLF, 256> : _Simd_testz<arch::ISA::AVX2, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VLBW, 256> : _Simd_testz<arch::ISA::AVX512VLF, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VLDQ, 256> : _Simd_testz<arch::ISA::AVX512VLF, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VLBWDQ, 256> : _Simd_testz<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMIVL, 256> : _Simd_testz<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI2VL, 256> : _Simd_testz<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMIVLDQ, 256> : _Simd_testz<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI2VLDQ, 256> : _Simd_testz<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Simd_testz<arch::ISA::AVX512VLF, 128> : _Simd_testz<arch::ISA::AVX2, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VLBW, 128> : _Simd_testz<arch::ISA::AVX512VLF, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VLDQ, 128> : _Simd_testz<arch::ISA::AVX512VLF, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VLBWDQ, 128> : _Simd_testz<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMIVL, 128> : _Simd_testz<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI2VL, 128> : _Simd_testz<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMIVLDQ, 128> : _Simd_testz<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Simd_testz<arch::ISA::AVX512VBMI2VLDQ, 128> : _Simd_testz<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
