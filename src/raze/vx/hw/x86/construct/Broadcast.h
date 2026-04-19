#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <raze/memory/PointerToIntegral.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_IntrinType_>
struct _Broadcast;

template <class _IntrinType_>
struct _Broadcast<arch::ISA::SSE2, 128, _IntrinType_> {
	template <class _Type_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_Type_ __value) raze_const_operator noexcept 
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm_set1_epi64x(memory::pointer_to_integral(__value)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm_set1_epi32(memory::pointer_to_integral(__value)));

		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm_set1_epi16(__value));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm_set1_epi8(__value));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm_set1_ps(__value));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm_set1_pd(__value));
	}
};

template <class _IntrinType_>
struct _Broadcast<arch::ISA::AVX, 256, _IntrinType_> {
	template <class _Type_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_Type_ __value) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm256_set1_epi64x(memory::pointer_to_integral(__value)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm256_set1_epi32(memory::pointer_to_integral(__value)));

		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm256_set1_epi16(__value));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm256_set1_epi8(__value));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm256_set1_ps(__value));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm256_set1_pd(__value));
	}
};

template <class _IntrinType_> 
struct _Broadcast<arch::ISA::AVX2, 256, _IntrinType_>: 
	_Broadcast<arch::ISA::AVX, 256, _IntrinType_>
{
	template <class _Type_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_Type_ __value) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
#if !defined(raze_os_win64)
			return __as<_IntrinType_>(_mm256_set1_epi64x(memory::pointer_to_integral(__value)));
#else
			return __as<_IntrinType_>(_mm256_broadcastq_epi64(
				_mm_cvtsi64_si128(memory::pointer_to_integral(__value))));
#endif // !defined(raze_os_win64)

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm256_broadcastd_epi32(
				_mm_cvtsi32_si128(memory::pointer_to_integral(__value))));

		else
			return _Broadcast<arch::ISA::AVX, 256, _IntrinType_>()(__value);
	}
};

template <class _IntrinType_> struct _Broadcast<arch::ISA::FMA3, 256, _IntrinType_> : _Broadcast<arch::ISA::AVX, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX2FMA3, 256, _IntrinType_> : _Broadcast<arch::ISA::AVX2, 256, _IntrinType_> {};

template <class _IntrinType_>
struct _Broadcast<arch::ISA::AVX512F, 512, _IntrinType_> {
	template <class _Type_>
	raze_nodiscard raze_static_operator raze_always_inline 
		_IntrinType_ operator()(_Type_ __value) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
#if !defined(raze_os_win64)
			return __as<_IntrinType_>(_mm512_set1_epi64(memory::pointer_to_integral(__value)));
#else
			return __as<_IntrinType_>(_mm512_broadcastq_epi64(_mm_cvtsi64_si128(memory::pointer_to_integral(__value))));
#endif

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm512_broadcastd_epi32(_mm_cvtsi32_si128(memory::pointer_to_integral(__value))));

		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm512_set1_epi16(__value));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm512_set1_epi8(__value));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm512_set1_ps(__value));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm512_set1_pd(__value));
	}
};

template <class _IntrinType_> struct _Broadcast<arch::ISA::SSE3, 128, _IntrinType_>: _Broadcast<arch::ISA::SSE2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::SSSE3, 128, _IntrinType_>: _Broadcast<arch::ISA::SSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::SSE41, 128, _IntrinType_>: _Broadcast<arch::ISA::SSSE3, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::SSE42, 128, _IntrinType_>: _Broadcast<arch::ISA::SSE41, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX, 128, _IntrinType_> : _Broadcast<arch::ISA::SSE42, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX2, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::FMA3, 128, _IntrinType_> : _Broadcast<arch::ISA::AVX, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX2FMA3, 128, _IntrinType_> : _Broadcast<arch::ISA::AVX2, 128, _IntrinType_> {};

template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512BW, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512DQ, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512F, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512BWDQ, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512BW, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI2, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512VBMI, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512BWDQ, 512, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI2DQ, 512, _IntrinType_>: _Broadcast<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_> {};

template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLF, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX2, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLBW, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLDQ, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLF, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMIVL, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLBW, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI2VL, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VBMIVL, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI2VLDQ, 256, _IntrinType_>: _Broadcast<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_> {};

template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLF, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX2, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLBW, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLDQ, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLF, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMIVL, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLBW, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI2VL, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VBMIVL, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_> {};
template <class _IntrinType_> struct _Broadcast<arch::ISA::AVX512VBMI2VLDQ, 128, _IntrinType_>: _Broadcast<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_> {};

__RAZE_VX_NAMESPACE_END
