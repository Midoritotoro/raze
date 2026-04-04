#pragma once 

#include <src/raze/datapar/arithmetic/Abs.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Mask_abs;

template <class _DesiredType_>
struct _Mask_abs<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Abs<arch::ISA::SSE2, 128, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Abs<arch::ISA::SSE2, 128, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}
};

template <class _DesiredType_>
struct _Mask_abs<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Abs<arch::ISA::AVX2, 256, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Abs<arch::ISA::AVX2, 256, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}
};

template <class _DesiredType_>
struct _Mask_abs<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
			_Abs<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
				__vector, __additional_source, __mask);
		}
		else if constexpr (__is_epi64_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_abs_epi64(
				__intrin_bitcast<__m512i>(__additional_source), __mask,
				__intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_abs_epi32(
				__intrin_bitcast<__m512i>(__additional_source), __mask,
				__intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_abs_ps(
				__intrin_bitcast<__m512>(__additional_source), __mask,
				__intrin_bitcast<__m512>(__vector)));
		}
		else if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_abs_pd(
				__intrin_bitcast<__m512d>(__additional_source), __mask,
				__intrin_bitcast<__m512d>(__vector)));
		}
		else {
			return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
				_Abs<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector),
				__additional_source, __mask);
		}
	}
};

template <class _DesiredType_>
struct _Mask_abs<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Mask_abs<arch::ISA::AVX512F, 512, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>()(
			_Abs<arch::ISA::AVX512BW, 512, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>()(
				__vector, __additional_source, __mask);
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_abs_epi8(
				__intrin_bitcast<__m512i>(__additional_source), __mask,
				__intrin_bitcast<__m512i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_abs_epi16(
				__intrin_bitcast<__m512i>(__additional_source), __mask,
				__intrin_bitcast<__m512i>(__vector)));
		}
		else {
			return _Mask_abs<arch::ISA::AVX512F, 512, _DesiredType_>()(
				__vector, __mask, __additional_source);
		}
	}
};

template <class _DesiredType_>
struct _Mask_abs<arch::ISA::AVX512VLF, 256, _DesiredType_> :
	_Mask_abs<arch::ISA::AVX2, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
			_Abs<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				__vector, __additional_source, __mask);
		}
		else if constexpr (__is_epi64_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_abs_epi64(
				__intrin_bitcast<__m256i>(__additional_source), __mask,
				__intrin_bitcast<__m256i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_abs_epi32(
				__intrin_bitcast<__m256i>(__additional_source), __mask,
				__intrin_bitcast<__m256i>(__vector)));
		}
		else {
			return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				_Abs<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__vector),
				__additional_source, __mask);
		}
	}
};

template <class _DesiredType_> 
struct _Mask_abs<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
	_Mask_abs<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
			_Abs<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
				__vector, __additional_source, __mask);
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_abs_epi8(
				__intrin_bitcast<__m256i>(__additional_source), __mask,
				__intrin_bitcast<__m256i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_abs_epi16(
				__intrin_bitcast<__m256i>(__additional_source), __mask,
				__intrin_bitcast<__m256i>(__vector)));
		}
		else {
			return _Mask_abs<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				__vector, __mask, __additional_source);
		}
	}
};

template <class _DesiredType_>
struct _Mask_abs<arch::ISA::AVX512VLF, 128, _DesiredType_> :
	_Mask_abs<arch::ISA::AVX2, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
			_Abs<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}


	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				__vector, __additional_source, __mask);
		}
		else if constexpr (__is_epi64_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_abs_epi64(
				__intrin_bitcast<__m128i>(__additional_source), __mask,
				__intrin_bitcast<__m128i>(__vector)));
		}
		else if constexpr (__is_epi32_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_abs_epi32(
				__intrin_bitcast<__m128i>(__additional_source), __mask,
				__intrin_bitcast<__m128i>(__vector)));
		}
		else {
			return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				_Abs<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__vector),
				__additional_source, __mask);
		}
	}
};

template <class _DesiredType_> 
struct _Mask_abs<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
	_Mask_abs<arch::ISA::AVX512VLF, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
			_Abs<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__vector),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (std::is_unsigned_v<_DesiredType_>) {
			return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
				__vector, __additional_source, __mask);
		}
		else if constexpr (__is_epi8_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_abs_epi8(
				__intrin_bitcast<__m128i>(__additional_source), __mask,
				__intrin_bitcast<__m128i>(__vector)));
		}
		else if constexpr (__is_epi16_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_abs_epi16(
				__intrin_bitcast<__m128i>(__additional_source), __mask,
				__intrin_bitcast<__m128i>(__vector)));
		}
		else {
			return _Mask_abs<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				__vector, __mask, __additional_source);
		}
	}
};


template <class _DesiredType_> struct _Mask_abs<arch::ISA::SSE3, 128, _DesiredType_> : _Mask_abs<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::SSSE3, 128, _DesiredType_> : _Mask_abs<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::SSE41, 128, _DesiredType_> : _Mask_abs<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::SSE42, 128, _DesiredType_> : _Mask_abs<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX2, 128, _DesiredType_> : _Mask_abs<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Mask_abs<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Mask_abs<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Mask_abs<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Mask_abs<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_abs<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Mask_abs<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
