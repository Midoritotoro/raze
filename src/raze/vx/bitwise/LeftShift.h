#pragma once 

#include <src/raze/vx/IntrinBitcast.h>
#include <src/raze/vx/shuffle/Blend.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Left_shift;

template <class _Type_>
struct _Left_shift<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm_sll_epi64(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm_sll_epi32(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_sll_epi16(
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __and_mask = _mm_and_si128(__as<__m128i>(__left), _mm_set1_epi8(0xFFu >> __shift));
			return __as<_IntrinType_>(_mm_sll_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
		}
	}
};

template <class _Type_>
struct _Left_shift<arch::ISA::AVX, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		const auto __low = _Left_shift<arch::ISA::SSE42, 128, _Type_>()(
			__as<__m128i>(__left), __shift);
		const auto __high = _Left_shift<arch::ISA::SSE42, 128, _Type_>()(
			_mm256_extractf128_si256(__as<__m256i>(__left), 1), __shift);

		return __as<_IntrinType_>(_mm256_insertf128_si256(
			__as<__m256i>(__low), __high, 1));
	}
};

template <class _Type_>
struct _Left_shift<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_sll_epi64(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_sll_epi32(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_sll_epi16(
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __and_mask = _mm256_and_si256(__as<__m256i>(__left), _mm256_set1_epi8(0xFFu >> __shift));
			return __as<_IntrinType_>(_mm256_sll_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
		}
	}
};

template <class _Type_>
struct _Left_shift<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_sll_epi64(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_sll_epi32(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			const auto __low = __as<__m256i>(__left);
			const auto __high = _mm512_extracti64x4_epi64(__as<__m512i>(__left), 1);

			const auto __low_shifted = _Left_shift<arch::ISA::AVX2, 256, _Type_>()(__low, __shift);
			const auto __high_shifted = _Left_shift<arch::ISA::AVX2, 256, _Type_>()(__high, __shift);

			return __as<_IntrinType_>(_mm512_inserti64x4(__as<__m512i>(__low_shifted), __high_shifted, 1));
		}
	}
};

template <class _Type_> 
struct _Left_shift<arch::ISA::AVX512BW, 512, _Type_>:
	_Left_shift<arch::ISA::AVX512F, 512, _Type_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift) raze_const_operator noexcept
	{
		if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __and_mask = _mm512_and_si512(__as<__m512i>(__left), _mm512_set1_epi8(0xFFull >> __shift));
			return __as<_IntrinType_>(_mm512_sll_epi16(__and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_sll_epi16(
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Left_shift<arch::ISA::AVX512F, 512, _Type_>()(__left, __shift);
		}
	}
};

template <class _Type_> struct _Left_shift<arch::ISA::SSE3, 128, _Type_> : _Left_shift<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::SSSE3, 128, _Type_> : _Left_shift<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::SSE41, 128, _Type_> : _Left_shift<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::SSE42, 128, _Type_> : _Left_shift<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX, 128, _Type_> : _Left_shift<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::FMA3, 128, _Type_> : _Left_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX2, 128, _Type_> : _Left_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX2FMA3, 128, _Type_> : _Left_shift<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Left_shift<arch::ISA::AVX512DQ, 512, _Type_> : _Left_shift<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512BWDQ, 512, _Type_> : _Left_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI, 512, _Type_> : _Left_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI2, 512, _Type_> : _Left_shift<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Left_shift<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Left_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Left_shift<arch::ISA::FMA3, 256, _Type_> : _Left_shift<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX2FMA3, 256, _Type_> : _Left_shift<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLF, 256, _Type_> : _Left_shift<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLBW, 256, _Type_> : _Left_shift<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLDQ, 256, _Type_> : _Left_shift<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Left_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Left_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Left_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Left_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Left_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLF, 128, _Type_> : _Left_shift<arch::ISA::AVX2, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLBW, 128, _Type_> : _Left_shift<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLDQ, 128, _Type_> : _Left_shift<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Left_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Left_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Left_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Left_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Left_shift<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Left_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
