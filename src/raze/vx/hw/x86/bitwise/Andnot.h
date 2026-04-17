#pragma once 

#include <src/raze/vx/IntrinBitcast.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Andnot;

template <>
struct _Andnot<arch::ISA::SSE2, 128> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m128d>)
			return _mm_andnot_pd(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m128i>)
			return _mm_andnot_si128(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m128>)
			return _mm_andnot_ps(__left, __right);
	}
};

template <> struct _Andnot<arch::ISA::AVX, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (std::is_same_v<_IntrinType_, __m256d>)
			return _mm256_andnot_pd(__left, __right);

		else
			return __as<_IntrinType_>(_mm256_andnot_ps(
				__as<__m256>(__left), __as<__m256>(__right)));
	}
}; 

template <>
struct _Andnot<arch::ISA::AVX2, 256> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if constexpr (std::is_same_v<_IntrinType_, __m256i>)
			return _mm256_andnot_si256(__left, __right);

		else
			return _Andnot<arch::ISA::AVX, 256>()(__left, __right);
	}
};

template <>
struct _Andnot<arch::ISA::AVX512F, 512> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
		if      constexpr (std::is_same_v<_IntrinType_, __m512d>)
			return _mm512_andnot_pd(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m512i>)
			return _mm512_andnot_si512(__left, __right);

		else if constexpr (std::is_same_v<_IntrinType_, __m512>)
			return _mm512_andnot_ps(__left, __right);
	}
};

template <> struct _Andnot<arch::ISA::SSE3, 128> : _Andnot<arch::ISA::SSE2, 128> {};
template <> struct _Andnot<arch::ISA::SSSE3, 128> : _Andnot<arch::ISA::SSE3, 128> {};
template <> struct _Andnot<arch::ISA::SSE41, 128> : _Andnot<arch::ISA::SSSE3, 128> {};
template <> struct _Andnot<arch::ISA::SSE42, 128> : _Andnot<arch::ISA::SSE41, 128> {};
template <> struct _Andnot<arch::ISA::AVX, 128> : _Andnot<arch::ISA::SSE42, 128> {};
template <> struct _Andnot<arch::ISA::FMA3, 128> : _Andnot<arch::ISA::AVX, 128> {};
template <> struct _Andnot<arch::ISA::AVX2, 128> : _Andnot<arch::ISA::AVX, 128> {};
template <> struct _Andnot<arch::ISA::AVX2FMA3, 128> : _Andnot<arch::ISA::AVX, 128> {};

template <> struct _Andnot<arch::ISA::AVX512BW, 512> : _Andnot<arch::ISA::AVX512F, 512> {};
template <> struct _Andnot<arch::ISA::AVX512DQ, 512> : _Andnot<arch::ISA::AVX512F, 512> {};
template <> struct _Andnot<arch::ISA::AVX512BWDQ, 512> : _Andnot<arch::ISA::AVX512BW, 512> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI, 512> : _Andnot<arch::ISA::AVX512BW, 512> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI2, 512> : _Andnot<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Andnot<arch::ISA::AVX512VBMIDQ, 512> : _Andnot<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI2DQ, 512> : _Andnot<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Andnot<arch::ISA::FMA3, 256> : _Andnot<arch::ISA::AVX, 256> {};
template <> struct _Andnot<arch::ISA::AVX2FMA3, 256> : _Andnot<arch::ISA::AVX, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VLF, 256> : _Andnot<arch::ISA::AVX2, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VLBW, 256> : _Andnot<arch::ISA::AVX512VLF, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VLDQ, 256> : _Andnot<arch::ISA::AVX512VLF, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VLBWDQ, 256> : _Andnot<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VBMIVL, 256> : _Andnot<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI2VL, 256> : _Andnot<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VBMIVLDQ, 256> : _Andnot<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI2VLDQ, 256> : _Andnot<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Andnot<arch::ISA::AVX512VLF, 128> : _Andnot<arch::ISA::AVX2, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VLBW, 128> : _Andnot<arch::ISA::AVX512VLF, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VLDQ, 128> : _Andnot<arch::ISA::AVX512VLF, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VLBWDQ, 128> : _Andnot<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VBMIVL, 128> : _Andnot<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI2VL, 128> : _Andnot<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VBMIVLDQ, 128> : _Andnot<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Andnot<arch::ISA::AVX512VBMI2VLDQ, 128> : _Andnot<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_VX_NAMESPACE_END
