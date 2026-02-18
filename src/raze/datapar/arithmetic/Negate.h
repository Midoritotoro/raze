#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Simd_negate;

template <class _DesiredType_>
struct _Simd_negate<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if      constexpr (__is_ps_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_xor_ps(__vector, _mm_set1_ps(-0.0f)));

		else if constexpr (__is_pd_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_xor_pd(__vector, __intrin_bitcast<__m128d>(_mm_setr_epi32(0, 0x80000000, 0, 0x80000000))));

		else
			return _Simd_sub<arch::ISA::SSE2, 128, _DesiredType_>()(_Simd_broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()(), __vector);
	}
};

template <class _DesiredType_>
struct _Simd_negate<arch::ISA::AVX2, 256, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if      constexpr (__is_ps_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_xor_ps(__intrin_bitcast<__m256>(__vector), _mm256_set1_ps(-0.0f)));

		else if constexpr (__is_pd_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_xor_pd(__intrin_bitcast<__m256d>(__vector),
				__intrin_bitcast<__m256d>(_mm256_setr_epi32(0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000))));

		else
			return _Simd_sub<arch::ISA::AVX2, 256, _DesiredType_>()(_Simd_broadcast_zeros<arch::ISA::AVX2, 256, _DesiredType_>()(), __vector);
	}
};

template <class _DesiredType_>
struct _Simd_negate<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(_IntrinType_ __vector) raze_const_operator noexcept {
		if      constexpr (__is_ps_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_xor_ps(__vector, _mm512_set1_ps(-0.0f)));

		else if constexpr (__is_pd_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_xor_pd(__vector, __intrin_bitcast<__m512d>(
				_mm512_setr_epi32(0, 0x80000000, 0, 0x80000000, 0, 0x80000000,
					0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000))));

		else
			return _Simd_sub<arch::ISA::AVX512F, 512, _DesiredType_>()(_Simd_broadcast_zeros<arch::ISA::AVX512F, 512, _DesiredType_>()(), __vector);
	}
};

template <class _DesiredType_> struct _Simd_negate<arch::ISA::SSE3, 128, _DesiredType_>: _Simd_negate<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::SSSE3, 128, _DesiredType_>: _Simd_negate<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::SSE41, 128, _DesiredType_>: _Simd_negate<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::SSE42, 128, _DesiredType_>: _Simd_negate<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX2, 128, _DesiredType_>: _Simd_negate<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512BW, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512DQ, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512BWDQ, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI2, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLF, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLBW, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLDQ, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMIVL, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLF, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLBW, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLDQ, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMIVL, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_negate<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_>: _Simd_negate<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
