#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/compare/Equal.h>
#include <src/raze/datapar/bitwise/ToBitmask.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Test_all_ones;

template <>
struct _Test_all_ones<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		bool operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		const auto __all_ones = _Equal<arch::ISA::SSE2, 128, int32>()(__vector, __vector);
		const auto __compared = _Equal<arch::ISA::SSE2, 128, int32>()(__vector, __all_ones);

		const auto __index_mask = _To_bitmask<arch::ISA::SSE2, 128, int32>()(__compared);
		return __index_mask == 0xF;
	}
};

template <> struct _Test_all_ones<arch::ISA::SSE3, 128> : _Test_all_ones<arch::ISA::SSE2, 128> {};
template <> struct _Test_all_ones<arch::ISA::SSSE3, 128> : _Test_all_ones<arch::ISA::SSE3, 128> {};

template <>
struct _Test_all_ones<arch::ISA::SSE41, 128> :
	_Test_all_ones<arch::ISA::SSSE3, 128>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		bool operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		const auto __all_ones = _Equal<arch::ISA::SSE41, 128, int64>()(__vector, __vector);
		return _mm_testc_si128(__intrin_bitcast<__m128i>(__vector), __intrin_bitcast<__m128i>(__all_ones));
	}
};


template <>
struct _Test_all_ones<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		bool operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		const auto __all_ones = _Equal<arch::ISA::AVX2, 256, int64>()(__vector, __vector);
		return _mm256_testc_si256(__intrin_bitcast<__m256i>(__vector), __intrin_bitcast<__m256i>(__all_ones));
	}
};

template <>
struct _Test_all_ones<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
		bool operator()(_IntrinType_ __vector) raze_const_operator noexcept
	{
		const auto __low = __intrin_bitcast<__m256i>(__vector);
		const auto __high = _mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1);

		using _Avx2Test = _Test_all_ones<arch::ISA::AVX2, 256>;
		return _Avx2Test()(__low) && _Avx2Test()(__high);
	}
};

template <> struct _Test_all_ones<arch::ISA::SSE42, 128> : _Test_all_ones<arch::ISA::SSE41, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX2, 128> : _Test_all_ones<arch::ISA::SSE42, 128> {};

template <> struct _Test_all_ones<arch::ISA::AVX512BW, 512> : _Test_all_ones<arch::ISA::AVX512F, 512> {};
template <> struct _Test_all_ones<arch::ISA::AVX512DQ, 512> : _Test_all_ones<arch::ISA::AVX512F, 512> {};
template <> struct _Test_all_ones<arch::ISA::AVX512BWDQ, 512> : _Test_all_ones<arch::ISA::AVX512BW, 512> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI, 512> : _Test_all_ones<arch::ISA::AVX512BW, 512> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI2, 512> : _Test_all_ones<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMIDQ, 512> : _Test_all_ones<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI2DQ, 512> : _Test_all_ones<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Test_all_ones<arch::ISA::AVX512VLF, 256> : _Test_all_ones<arch::ISA::AVX2, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VLBW, 256> : _Test_all_ones<arch::ISA::AVX512VLF, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VLDQ, 256> : _Test_all_ones<arch::ISA::AVX512VLF, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VLBWDQ, 256> : _Test_all_ones<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMIVL, 256> : _Test_all_ones<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI2VL, 256> : _Test_all_ones<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMIVLDQ, 256> : _Test_all_ones<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI2VLDQ, 256> : _Test_all_ones<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Test_all_ones<arch::ISA::AVX512VLF, 128> : _Test_all_ones<arch::ISA::AVX2, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VLBW, 128> : _Test_all_ones<arch::ISA::AVX512VLF, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VLDQ, 128> : _Test_all_ones<arch::ISA::AVX512VLF, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VLBWDQ, 128> : _Test_all_ones<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMIVL, 128> : _Test_all_ones<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI2VL, 128> : _Test_all_ones<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMIVLDQ, 128> : _Test_all_ones<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Test_all_ones<arch::ISA::AVX512VBMI2VLDQ, 128> : _Test_all_ones<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_DATAPAR_NAMESPACE_END
