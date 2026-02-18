#pragma once 

#include <src/raze/datapar/memory/Load.h>
#include <src/raze/datapar/memory/AlignmentPolicy.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_IntrinType_>
struct _Simd_non_temporal_load;

template <class _IntrinType_>
struct _Simd_non_temporal_load<arch::ISA::SSE2, 128, _IntrinType_> {
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(const void* __address) raze_const_operator noexcept 
	{
		return _Simd_load<arch::ISA::SSE2, 128, _IntrinType_>()(__address, __aligned_policy{});
	}
};

template <class _IntrinType_> 
struct _Simd_non_temporal_load<arch::ISA::SSE41, 128, _IntrinType_>:
	_Simd_non_temporal_load<arch::ISA::SSSE3, 128, _IntrinType_> 
{
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(const void* __address) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm_stream_load_si128(reinterpret_cast<const __m128i*>(__address)));
	}
};

template <class _IntrinType_>
struct _Simd_non_temporal_load<arch::ISA::AVX2, 256, _IntrinType_> {
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(const void* __address) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm256_stream_load_si256(reinterpret_cast<const __m256i*>(__address)));
	}
};

template <class _IntrinType_>
struct _Simd_non_temporal_load<arch::ISA::AVX512F, 512, _IntrinType_> {
	raze_nodiscard raze_static_operator raze_always_inline
		_IntrinType_ operator()(const void* __address) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm512_stream_load_si512(__address));
	}
};

template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::SSE3, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::SSE2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::SSSE3, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::SSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::SSE42, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::SSE41, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX2, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::SSE42, 128, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512BW, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512DQ, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512BWDQ, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI2, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VBMI, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512BWDQ, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI2DQ, 512, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLF, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX2, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLBW, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLDQ, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMIVL, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI2VL, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VBMIVL, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI2VLDQ, 256, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLF, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLBW, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLDQ, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMIVL, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI2VL, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VBMIVL, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_non_temporal_load<arch::ISA::AVX512VBMI2VLDQ, 128, _IntrinType_> : _Simd_non_temporal_load<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_> {};

__RAZE_DATAPAR_NAMESPACE_END
