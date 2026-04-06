#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/bitwise/LeftShift.h>
#include <src/raze/datapar/arithmetic/MaskzAssign.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Maskz_left_shift;

template <class _DesiredType_>
struct _Maskz_left_shift<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Left_shift<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __shift), __mask);
	}
};

template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::SSE3, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::SSSE3, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::SSE41, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::SSE42, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX2, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Maskz_left_shift<arch::ISA::AVX512VLF, 128, _DesiredType_>: 
	_Maskz_left_shift<arch::ISA::AVX2, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_sll_epi64(__mask,
				__intrin_bitcast<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_sll_epi32(__mask,
				__intrin_bitcast<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else 
			return _Maskz_assign<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				_Left_shift<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __shift), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_left_shift<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
	_Maskz_left_shift<arch::ISA::AVX512VLF, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
			const auto __and_mask = _mm_and_si128(__intrin_bitcast<__m128i>(__left), _mm_set1_epi8(0xFFull >> __shift));
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_sll_epi16(__mask, __and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_sll_epi16(__mask,
				__intrin_bitcast<__m128i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_left_shift<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __shift, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_left_shift<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __shift), __mask);
	}
};

template <class _DesiredType_> 
struct _Maskz_left_shift<arch::ISA::AVX512VLF, 256, _DesiredType_>:
	_Maskz_left_shift<arch::ISA::AVX2, 256, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_sll_epi64(__mask,
				__intrin_bitcast<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_sll_epi32(__mask,
				__intrin_bitcast<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else 
			return _Maskz_assign<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				_Left_shift<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __shift), __mask);
	}
};

template <class _DesiredType_> 
struct _Maskz_left_shift<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
	_Maskz_left_shift<arch::ISA::AVX512VLF, 256, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
			const auto __and_mask = _mm256_and_si256(__intrin_bitcast<__m256i>(__left), _mm256_set1_epi8(0xFFull >> __shift));
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_sll_epi16(__mask, __and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_sll_epi16(__mask,
				__intrin_bitcast<__m256i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_left_shift<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __shift, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Maskz_left_shift<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512F, 512, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_sll_epi64(__mask,
				__intrin_bitcast<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_sll_epi32(__mask,
				__intrin_bitcast<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		
		else 
			return _Maskz_assign<arch::ISA::AVX512F, 512, _DesiredType_>()(
				_Left_shift<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __shift), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_left_shift<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Maskz_left_shift<arch::ISA::AVX512F, 512, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512BW, 512, _DesiredType_>()(
			_Left_shift<arch::ISA::AVX512BW, 512, _DesiredType_>()(__left, __shift), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		uint32			__shift,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
			const auto __and_mask = _mm512_and_si512(__intrin_bitcast<__m512i>(__left), _mm512_set1_epi8(0xFFull >> __shift));
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_sll_epi16(__mask, __and_mask, _mm_cvtsi32_si128(__shift)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_sll_epi16(__mask,
				__intrin_bitcast<__m512i>(__left), _mm_cvtsi32_si128(__shift)));
		}
		else {
			return _Maskz_left_shift<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __shift, __mask);
		}
	}
};

template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_left_shift<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Maskz_left_shift<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
