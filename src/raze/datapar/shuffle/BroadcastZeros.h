#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_IntrinType_>
struct _Simd_broadcast_zeros;

template <class _IntrinType_>
struct _Simd_broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_> {
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()() raze_const_operator noexcept {
		if constexpr (std::is_same_v<_IntrinType_, __m128i>)
			return _mm_setzero_si128();

		else if constexpr (std::is_same_v<_IntrinType_, __m128d>)
			return _mm_setzero_pd();

		else if constexpr (std::is_same_v<_IntrinType_, __m128>)
			return _mm_setzero_ps();
	}
};

template <class _IntrinType_>
struct _Simd_broadcast_zeros<arch::ISA::AVX2, 256, _IntrinType_> {
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()() raze_const_operator noexcept {
		if constexpr (std::is_same_v<_IntrinType_, __m256i>)
			return _mm256_setzero_si256();

		else if constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_setzero_pd();

		else if constexpr (std::is_same_v<_IntrinType_, __m256>)
			return _mm256_setzero_ps();
	}
};

template <class _IntrinType_>
struct _Simd_broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_> {
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()() raze_const_operator noexcept {
		if constexpr (std::is_same_v<_IntrinType_, __m512i>)
			return _mm512_setzero_si512();

		else if constexpr (std::is_same_v<_IntrinType_, __m512d>)
			return _mm512_setzero_pd();

		else if constexpr (std::is_same_v<_IntrinType_, __m512>)
			return _mm512_setzero_ps();
	}
};

template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::SSE3, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::SSSE3, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::SSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::SSE41, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::SSSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::SSE42, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::SSE41, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX2, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::SSE42, 128, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512BW, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512DQ, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512BWDQ, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI2, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VBMI, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512BWDQ, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI2DQ, 512, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX2, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLBW, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLDQ, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVL, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI2VL, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVL, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI2VLDQ, 256, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_> {};

template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLF, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLBW, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLDQ, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVL, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI2VL, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVL, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Simd_broadcast_zeros<arch::ISA::AVX512VBMI2VLDQ, 128, _IntrinType_>: _Simd_broadcast_zeros<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_> {};

__RAZE_DATAPAR_NAMESPACE_END
