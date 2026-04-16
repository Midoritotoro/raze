#pragma once 

#include <src/raze/vx/arithmetic/Sub.h>
#include <src/raze/vx/shuffle/BroadcastZeros.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Negate;

template <class _Type_>
struct _Negate<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if      constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm_xor_ps(__vector, 
				__as<__m128>(_mm_set1_epi32(0x80000000))));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm_xor_pd(__vector, 
				__as<__m128d>(_mm_setr_epi32(0, 0x80000000, 0, 0x80000000))));

		else
			return _Sub<arch::ISA::SSE2, 128, _Type_>()(
				_Broadcast_zeros<arch::ISA::SSE2, 128, _IntrinType_>()(), __vector);
	}
};

template <class _Type_> 
struct _Negate<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if      constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm256_xor_ps(__as<__m256>(__vector), 
				__as<__m256>(_mm256_set1_epi32(0x80000000))));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm256_xor_pd(__as<__m256d>(__vector),
				__as<__m256d>(_mm256_setr_epi32(0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000))));

		else
			return _Sub<arch::ISA::AVX, 256, _Type_>()(
				_Broadcast_zeros<arch::ISA::AVX, 256, _IntrinType_>()(), __vector);
	}
};

template <class _Type_>
struct _Negate<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_
		operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if      constexpr (__is_ps_v<_Type_> || __is_pd_v<_Type_>)
			return _Negate<arch::ISA::AVX, 256, _Type_>()(__vector);

		else
			return _Sub<arch::ISA::AVX2, 256, _Type_>()(
				_Broadcast_zeros<arch::ISA::AVX2, 256, _IntrinType_>()(), __vector);
	}
};

template <class _Type_>
struct _Negate<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ 
		operator()(_IntrinType_ __vector) raze_const_operator noexcept 
	{
		if      constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm512_xor_ps(__vector, 
				__as<__m512>(_mm512_set1_epi32(0x80000000))));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm512_xor_pd(__vector, __as<__m512d>(
				_mm512_setr_epi32(0, 0x80000000, 0, 0x80000000, 0, 0x80000000,
					0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000, 0, 0x80000000))));

		else
			return _Sub<arch::ISA::AVX512F, 512, _Type_>()(
				_Broadcast_zeros<arch::ISA::AVX512F, 512, _IntrinType_>()(), __vector);
	}
};

template <class _Type_> struct _Negate<arch::ISA::SSE3, 128, _Type_>: _Negate<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::SSSE3, 128, _Type_>: _Negate<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::SSE41, 128, _Type_>: _Negate<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::SSE42, 128, _Type_>: _Negate<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX, 128, _Type_>: _Negate<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX2, 128, _Type_> : _Negate<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::FMA3, 128, _Type_> : _Negate<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX2FMA3, 128, _Type_> : _Negate<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Negate<arch::ISA::AVX512BW, 512, _Type_>: _Negate<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512DQ, 512, _Type_>: _Negate<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512BWDQ, 512, _Type_>: _Negate<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI, 512, _Type_>: _Negate<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI2, 512, _Type_>: _Negate<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMIDQ, 512, _Type_>: _Negate<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI2DQ, 512, _Type_>: _Negate<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Negate<arch::ISA::FMA3, 256, _Type_> : _Negate<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX2FMA3, 256, _Type_> : _Negate<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLF, 256, _Type_>: _Negate<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLBW, 256, _Type_>: _Negate<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLDQ, 256, _Type_>: _Negate<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLBWDQ, 256, _Type_>: _Negate<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMIVL, 256, _Type_>: _Negate<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI2VL, 256, _Type_>: _Negate<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMIVLDQ, 256, _Type_>: _Negate<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_>: _Negate<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Negate<arch::ISA::AVX512VLF, 128, _Type_>: _Negate<arch::ISA::AVX2, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLBW, 128, _Type_>: _Negate<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLDQ, 128, _Type_>: _Negate<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VLBWDQ, 128, _Type_>: _Negate<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMIVL, 128, _Type_>: _Negate<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI2VL, 128, _Type_>: _Negate<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMIVLDQ, 128, _Type_>: _Negate<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Negate<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_>: _Negate<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
