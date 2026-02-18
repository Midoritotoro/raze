#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Simd_non_temporal_store;

template <>
struct _Simd_non_temporal_store<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_static_operator raze_always_inline void operator()(
		const void*		__address,
		_IntrinType_	__vector) raze_const_operator noexcept
	{
		_mm_stream_si128(reinterpret_cast<__m128i*>(__address), __intrin_bitcast<__m128i>(__vector));
	}
};

template <>
struct _Simd_non_temporal_store<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_static_operator raze_always_inline void operator()(
		const void*		__address,
		_IntrinType_	__vector) raze_const_operator noexcept
	{
		_mm256_stream_si256(reinterpret_cast<__m128i*>(__address), __intrin_bitcast<__m128i>(__vector));
	}
};

template <>
struct _Simd_non_temporal_store<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_static_operator raze_always_inline void operator()(
		const void* __address,
		_IntrinType_	__vector) raze_const_operator noexcept
	{
		_mm512_stream_si512(__address, __intrin_bitcast<__m512i>(__vector));
	}
};

template <> struct _Simd_non_temporal_store<arch::ISA::SSE3, 128> : _Simd_non_temporal_store<arch::ISA::SSE2, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::SSSE3, 128> : _Simd_non_temporal_store<arch::ISA::SSE3, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::SSE41, 128> : _Simd_non_temporal_store<arch::ISA::SSSE3, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::SSE42, 128> : _Simd_non_temporal_store<arch::ISA::SSE41, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX2, 128> : _Simd_non_temporal_store<arch::ISA::SSE42, 128> {};

template <> struct _Simd_non_temporal_store<arch::ISA::AVX512BW, 512> : _Simd_non_temporal_store<arch::ISA::AVX512F, 512> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512DQ, 512> : _Simd_non_temporal_store<arch::ISA::AVX512F, 512> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512BWDQ, 512> : _Simd_non_temporal_store<arch::ISA::AVX512BW, 512> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI, 512> : _Simd_non_temporal_store<arch::ISA::AVX512BW, 512> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI2, 512> : _Simd_non_temporal_store<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMIDQ, 512> : _Simd_non_temporal_store<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI2DQ, 512> : _Simd_non_temporal_store<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLF, 256> : _Simd_non_temporal_store<arch::ISA::AVX2, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLBW, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VLF, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLDQ, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VLF, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLBWDQ, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMIVL, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI2VL, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMIVLDQ, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI2VLDQ, 256> : _Simd_non_temporal_store<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLF, 128> : _Simd_non_temporal_store<arch::ISA::AVX2, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLBW, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VLF, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLDQ, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VLF, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VLBWDQ, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMIVL, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI2VL, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMIVLDQ, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Simd_non_temporal_store<arch::ISA::AVX512VBMI2VLDQ, 128> : _Simd_non_temporal_store<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
