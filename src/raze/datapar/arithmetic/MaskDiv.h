#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/Blend.h>
#include <src/raze/datapar/arithmetic/Div.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Mask_div;

template <class _DesiredType_>
struct _Mask_div<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Div<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE2, 128, _DesiredType_>()(
			_Div<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_> struct _Mask_div<arch::ISA::SSE3, 128, _DesiredType_> : _Mask_div<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::SSSE3, 128, _DesiredType_> : _Mask_div<arch::ISA::SSE3, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Mask_div<arch::ISA::SSE41, 128, _DesiredType_>:
	_Mask_div<arch::ISA::SSSE3, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE41, 128, _DesiredType_>()(
			_Div<arch::ISA::SSE41, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::SSE41, 128, _DesiredType_>()(
			_Div<arch::ISA::SSE41, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_>
struct _Mask_div<arch::ISA::AVX512VLF, 128, _DesiredType_>:
	_Mask_div<arch::ISA::AVX2, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_div_pd(
				__intrin_bitcast<__m128d>(__additional_source), __mask, 
				__intrin_bitcast<__m128d>(__left), _mm_set1_pd(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_div_ps(
				__intrin_bitcast<__m128>(__additional_source), __mask,
				__intrin_bitcast<__m128>(__left), _mm_set1_ps(__right)));
		}
		else {
			return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_div_pd(
				__intrin_bitcast<__m128d>(__additional_source), __mask,
				__intrin_bitcast<__m128d>(__left), __intrin_bitcast<__m128d>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm_mask_div_ps(
				__intrin_bitcast<__m128>(__additional_source), __mask,
				__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__right)));
		}
		else {
			return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};


template <class _DesiredType_>
struct _Mask_div<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
	_Mask_div<arch::ISA::AVX512VLF, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_> || __is_ps_v<_DesiredType_>) {
			return _Mask_div<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				__left, __right, __mask, __additional_source);
		}
		else {
			return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_> || __is_ps_v<_DesiredType_>) {
			return _Mask_div<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				__left, __right, __mask, __additional_source);
		}
		else {
			return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_>
struct _Mask_div<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Div<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX2, 256, _DesiredType_>()(
			_Div<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_> 
struct _Mask_div<arch::ISA::AVX512VLF, 256, _DesiredType_>:
	_Mask_div<arch::ISA::AVX2, 256, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_div_pd(
				__intrin_bitcast<__m256d>(__additional_source), __mask, 
				__intrin_bitcast<__m256d>(__left), _mm256_set1_pd(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_div_ps(
				__intrin_bitcast<__m256>(__additional_source), __mask,
				__intrin_bitcast<__m256>(__left), _mm256_set1_ps(__right)));
		}
		else {
			return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_div_pd(
				__intrin_bitcast<__m256d>(__additional_source), __mask,
				__intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm256_mask_div_ps(
				__intrin_bitcast<__m256>(__additional_source), __mask,
				__intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right)));
		}
		else {
			return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_> 
struct _Mask_div<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
	_Mask_div<arch::ISA::AVX512VLF, 256, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_> || __is_ps_v<_DesiredType_>) {
			return _Mask_div<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				__left, __right, __mask, __additional_source);
		}
		else {
			return _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_> || __is_ps_v<_DesiredType_>) {
			return _Mask_div<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				__left, __right, __mask, __additional_source);
		}
		else {
			return _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
				_Div<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
			_Div<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_>
struct _Mask_div<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_div_pd(
				__intrin_bitcast<__m512d>(__additional_source), __mask, 
				__intrin_bitcast<__m512d>(__left), _mm512_set1_pd(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_div_ps(
				__intrin_bitcast<__m512>(__additional_source), __mask,
				__intrin_bitcast<__m512>(__left), _mm512_set1_ps(__right)));
		}
		else {
			return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
				_Div<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_div_pd(
				__intrin_bitcast<__m512d>(__additional_source), __mask,
				__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right)));
		}
		else if constexpr (__is_ps_v<_DesiredType_>) {
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_div_ps(
				__intrin_bitcast<__m512>(__additional_source), __mask,
				__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right)));
		}
		else {
			return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
				_Div<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
			_Div<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(
			_Div<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_> 
struct _Mask_div<arch::ISA::AVX512BW, 512, _DesiredType_>:
	_Mask_div<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_> || __is_ps_v<_DesiredType_>) {
			return _Mask_div<arch::ISA::AVX512F, 512, _DesiredType_>()(
				__left, __right, __mask, __additional_source);
		}
		else {
			return _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>()(
				_Div<arch::ISA::AVX512BW, 512, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_DesiredType_> || __is_ps_v<_DesiredType_>) {
			return _Mask_div<arch::ISA::AVX512F, 512, _DesiredType_>()(
				__left, __right, __mask, __additional_source);
		}
		else {
			return _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>()(
				_Div<arch::ISA::AVX512BW, 512, _DesiredType_>()(__left, __right),
				__additional_source, __mask);
		}
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_DesiredType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>()(
			_Div<arch::ISA::AVX512BW, 512, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask,
		_IntrinType_	__additional_source) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>()(
			_Div<arch::ISA::AVX512BW, 512, _DesiredType_>()(__left, __right),
			__additional_source, __mask);
	}
};

template <class _DesiredType_> struct _Mask_div<arch::ISA::SSE42, 128, _DesiredType_> : _Mask_div<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX2, 128, _DesiredType_> : _Mask_div<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Mask_div<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Mask_div<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Mask_div<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Mask_div<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Mask_div<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Mask_div<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Mask_div<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Mask_div<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Mask_div<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Mask_div<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Mask_div<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Mask_div<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
