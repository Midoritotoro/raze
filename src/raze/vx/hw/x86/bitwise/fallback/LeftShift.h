#pragma once 

#include <src/raze/vx/IntrinBitcast.h>
#include <src/raze/vx/bitwise/LeftShift.h>
#include <src/raze/vx/arithmetic/MaskzAssign.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Mask_left_shift;

template <class _Type_>
struct _Mask_left_shift<arch::ISA::SSE2, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
	{
		return _Blend<arch::ISA::SSE2, 128, _Type_>()(
			_Left_shift<arch::ISA::SSE2, 128, _Type_>()(__left, __shift), 
			__source, __mask);
	}
};

template <class _Type_> struct _Mask_left_shift<arch::ISA::SSE3, 128, _Type_> : _Mask_left_shift<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::SSSE3, 128, _Type_> : _Mask_left_shift<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::SSE41, 128, _Type_> : _Mask_left_shift<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::SSE42, 128, _Type_> : _Mask_left_shift<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX, 128, _Type_> : _Mask_left_shift<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX2, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::FMA3, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX2FMA3, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX2, 128, _Type_> {};


template <class _Type_> 
struct _Mask_left_shift<arch::ISA::AVX512VLF, 128, _Type_>: 
	_Mask_left_shift<arch::ISA::AVX2, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 128, _Type_>()(
			_Left_shift<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __shift),
			__source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm_mask_sll_epi64(
				__as<__m128i>(__source), __mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm_mask_sll_epi32(
				__as<__m128i>(__source), __mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else 
			return _Blend<arch::ISA::AVX512VLF, 128, _Type_>()(
				_Left_shift<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __shift),
				__source, __mask);
	}
};

template <class _Type_>
struct _Mask_left_shift<arch::ISA::AVX512VLBW, 128, _Type_>:
	_Mask_left_shift<arch::ISA::AVX512VLF, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 128, _Type_>()(
			_Left_shift<arch::ISA::AVX512VLBW, 128, _Type_>()(__left, __shift),
			__source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __and_mask = _mm_and_si128(__as<__m128i>(__left), _mm_set1_epi8(0xFFull >> __shift));
			return __as<_IntrinType_>(_mm_mask_sll_epi16(
				__as<__m128i>(__source), __mask, 
				__and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm_mask_sll_epi16(
				__as<__m128i>(__source), __mask,
				__as<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Mask_left_shift<arch::ISA::AVX512VLF, 128, _Type_>()(
				__left, __shift, __mask, __source);
		}
	}
};

template <class _Type_> 
struct _Mask_left_shift<arch::ISA::AVX, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
	{
		return _Blend<arch::ISA::AVX, 256, _Type_>()(
			_Left_shift<arch::ISA::AVX, 256, _Type_>()(__left, __shift),
			__source, __mask);
	}
};

template <class _Type_>
struct _Mask_left_shift<arch::ISA::AVX2, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
	{
		return _Blend<arch::ISA::AVX2, 256, _Type_>()(
			_Left_shift<arch::ISA::AVX2, 256, _Type_>()(__left, __shift),
			__source, __mask);
	}
};

template <class _Type_> struct _Mask_left_shift<arch::ISA::FMA3, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX2FMA3, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX2, 256, _Type_> {};

template <class _Type_> 
struct _Mask_left_shift<arch::ISA::AVX512VLF, 256, _Type_>:
	_Mask_left_shift<arch::ISA::AVX2, 256, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 256, _Type_>()(
			_Left_shift<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __shift),
			__source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm256_mask_sll_epi64(
				__as<__m256i>(__source), __mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm256_mask_sll_epi32(
				__as<__m256i>(__source), __mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else 
			return _Blend<arch::ISA::AVX512VLF, 256, _Type_>()(
				_Left_shift<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __shift),
				__source, __mask);
	}
};

template <class _Type_> 
struct _Mask_left_shift<arch::ISA::AVX512VLBW, 256, _Type_>:
	_Mask_left_shift<arch::ISA::AVX512VLF, 256, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 256, _Type_>()(
			_Left_shift<arch::ISA::AVX512VLBW, 256, _Type_>()(__left, __shift),
			__source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __and_mask = _mm256_and_si256(__as<__m256i>(__left), _mm256_set1_epi8(0xFFull >> __shift));
			return __as<_IntrinType_>(_mm256_mask_sll_epi16(
				__as<__m256i>(__source), __mask,
				__and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm256_mask_sll_epi16(
				__as<__m256i>(__source), __mask,
				__as<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Mask_left_shift<arch::ISA::AVX512VLF, 256, _Type_>()(
				__left, __shift, __mask, __source);
		}
	}
};

template <class _Type_>
struct _Mask_left_shift<arch::ISA::AVX512F, 512, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512F, 512, _Type_>()(
			_Left_shift<arch::ISA::AVX512F, 512, _Type_>()(__left, __shift),
			__source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm512_mask_sll_epi64(
				__as<__m512i>(__source), __mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm512_mask_sll_epi32(
				__as<__m512i>(__source), __mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else 
			return _Blend<arch::ISA::AVX512F, 512, _Type_>()(
				_Left_shift<arch::ISA::AVX512F, 512, _Type_>()(__left, __shift),
				__source, __mask);
	}
};

template <class _Type_>
struct _Mask_left_shift<arch::ISA::AVX512BW, 512, _Type_> :
	_Mask_left_shift<arch::ISA::AVX512F, 512, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512BW, 512, _Type_>()(
			_Left_shift<arch::ISA::AVX512BW, 512, _Type_>()(__left, __shift),
			__source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask,
		_IntrinType_	__source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
			const auto __and_mask = _mm512_and_si512(__as<__m512i>(__left), _mm512_set1_epi8(0xFFull >> __shift));
			return __as<_IntrinType_>(_mm512_mask_sll_epi16(
				__as<__m512i>(__source), __mask, 
				__and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
			return __as<_IntrinType_>(_mm512_mask_sll_epi16(
				__as<__m512i>(__source), __mask,
				__as<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Mask_left_shift<arch::ISA::AVX512F, 512, _Type_>()(
				__left, __shift, __mask, __source);
		}
	}
};

template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512DQ, 512, _Type_> : _Mask_left_shift<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512BWDQ, 512, _Type_> : _Mask_left_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI, 512, _Type_> : _Mask_left_shift<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI2, 512, _Type_> : _Mask_left_shift<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Mask_left_shift<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Mask_left_shift<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VLDQ, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Mask_left_shift<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VLDQ, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Mask_left_shift<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Mask_left_shift<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
