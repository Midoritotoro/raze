#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/MaskExpand.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_blend;

template <class _DesiredType_>
struct _Simd_blend<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __first,
		_IntrinType_ __second,
		_IntrinType_ __mask) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm_or_si128(
			_mm_and_si128(__intrin_bitcast<__m128i>(__mask), __intrin_bitcast<__m128i>(__first)),
			_mm_andnot_si128(__intrin_bitcast<__m128i>(__mask), __intrin_bitcast<__m128i>(__second))));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Simd_blend()(__first, __second, _Simd_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_> struct _Simd_blend<arch::ISA::SSE3, 128, _DesiredType_>: _Simd_blend<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::SSSE3, 128, _DesiredType_>: _Simd_blend<arch::ISA::SSE3, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Simd_blend<arch::ISA::SSE41, 128, _DesiredType_>:
	_Simd_blend<arch::ISA::SSSE3, 128, _DesiredType_>
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __first,
		_IntrinType_ __second,
		_IntrinType_ __mask) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm_blendv_epi8(__intrin_bitcast<__m128i>(__second),
			__intrin_bitcast<__m128i>(__first), __intrin_bitcast<__m128i>(__mask)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Simd_blend()(__first, __second, _Simd_to_vector<arch::ISA::SSE41, 128, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_>
struct _Simd_blend<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __first,
		_IntrinType_ __second,
		_IntrinType_ __mask) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm256_blendv_epi8(__intrin_bitcast<__m256i>(__second),
			__intrin_bitcast<__m256i>(__first), __intrin_bitcast<__m256i>(__mask)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Simd_blend()(__first, __second, _Simd_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_>
struct _Simd_blend<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __first,
		_IntrinType_ __second,
		_IntrinType_ __mask) raze_const_operator noexcept
	{
		return __intrin_bitcast<_IntrinType_>(_mm512_ternarylogic_epi32(__intrin_bitcast<__m512i>(__mask),
			__intrin_bitcast<__m512i>(__first), __intrin_bitcast<__m512i>(__second), 0xCA));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Simd_blend()(__first, __second, _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_>
struct _Simd_blend<arch::ISA::AVX512BW, 512, _DesiredType_>: 
	_Simd_blend<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __first,
		_IntrinType_ __second,
		_IntrinType_ __mask) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) == 2)
			return __intrin_bitcast<_IntrinType_>(_mm512_ternarylogic_epi32(
				__intrin_bitcast<__m512i>(__mask), __intrin_bitcast<__m512i>(__first), __intrin_bitcast<__m512i>(__second), 0xCA));

		else
			return _Simd_blend()(__first, __second, _Simd_to_mask<arch::ISA::AVX512BW, 512, _DesiredType_>()(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		if constexpr (sizeof(_DesiredType_) == 2)
			return _Simd_blend()(__first, __second, _Simd_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_>()(__mask));

		else
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_blend_epi8(
				__expand_mask_bits_zmm(__mask), __intrin_bitcast<__m512i>(__second),
				__intrin_bitcast<__m512i>(__first)));
	}
};

template <class _DesiredType_> struct _Simd_blend<arch::ISA::SSE42, 128, _DesiredType_>: _Simd_blend<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX2, 128, _DesiredType_>: _Simd_blend<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512DQ, 512, _DesiredType_>: _Simd_blend<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512BWDQ, 512, _DesiredType_>: _Simd_blend<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI, 512, _DesiredType_>: _Simd_blend<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI2, 512, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_>: _Simd_blend<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLF, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLDQ, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMIVL, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLF, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLDQ, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMIVL, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_blend<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_>: _Simd_blend<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
