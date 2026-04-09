#pragma once 

#include <src/raze/datapar/bitwise/BitAnd.h>
#include <src/raze/datapar/arithmetic/MaskzAssign.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Maskz_and;

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _DesiredType_>()(
			_And<arch::ISA::SSE2, 128>()(__left, __right), __mask);
	}
};


template <class _DesiredType_> struct _Maskz_and<arch::ISA::SSE3, 128, _DesiredType_> : _Maskz_and<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::SSSE3, 128, _DesiredType_> : _Maskz_and<arch::ISA::SSE3, 128, _DesiredType_> {};


template <class _DesiredType_>
struct _Maskz_and<arch::ISA::SSE41, 128, _DesiredType_> :
	_Maskz_and<arch::ISA::SSSE3, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::SSE41, 128, _DesiredType_>()(
			_And<arch::ISA::SSE41, 128>()(__left, __right), __mask);
	}
};

template <class _DesiredType_> struct _Maskz_and<arch::ISA::SSE42, 128, _DesiredType_> : _Maskz_and<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX, 128, _DesiredType_> : _Maskz_and<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX2, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::FMA3, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX2, 128, _DesiredType_> {};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX512VLF, 128, _DesiredType_> :
	_Maskz_and<arch::ISA::AVX2, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
			_And<arch::ISA::AVX512VLF, 128>()(__left, __right), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) == 8)
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_and_epi64(__mask,
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return __intrin_bitcast<_IntrinType_>(_mm_maskz_and_epi32(__mask,
				__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));

		else
			return _Maskz_assign<arch::ISA::AVX512VLF, 128, _DesiredType_>()(
				_And<arch::ISA::AVX512VLF, 128>()(__left, __right), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX512VLBW, 128, _DesiredType_> :
	_Maskz_and<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
			_And<arch::ISA::AVX512VLBW, 128>()(__left, __right), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Maskz_and<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __right, __mask);
		else
			return _Maskz_assign<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(
				_And<arch::ISA::AVX512VLBW, 128>()(__left, __right), __mask);
	}
};

template <class _DesiredType_> 
struct _Maskz_and<arch::ISA::AVX, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX, 256, _DesiredType_>()(
			_And<arch::ISA::AVX, 256>()(__left, __right), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
	{
		return _Maskz_assign<arch::ISA::AVX2, 256, _DesiredType_>()(
			_And<arch::ISA::AVX2, 256>()(__left, __right), __mask);
	}
};

template <class _DesiredType_> struct _Maskz_and<arch::ISA::FMA3, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX2FMA3, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX2, 256, _DesiredType_> {};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX512VLF, 256, _DesiredType_> :
	_Maskz_and<arch::ISA::AVX2, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
			_And<arch::ISA::AVX512VLF, 256>()(__left, __right), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_and_epi64(__mask,
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));

		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm256_maskz_and_epi32(__mask,
				__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right)));

		else
			return _Maskz_assign<arch::ISA::AVX512VLF, 256, _DesiredType_>()(
				_And<arch::ISA::AVX512VLF, 256>()(__left, __right), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX512VLBW, 256, _DesiredType_> :
	_Maskz_and<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
			_And<arch::ISA::AVX512VLBW, 256>()(__left, __right), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Maskz_and<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right, __mask);
		else
			return _Maskz_assign<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(
				_And<arch::ISA::AVX512VLBW, 256>()(__left, __right), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512F, 512, _DesiredType_>()(
			_And<arch::ISA::AVX512F, 512>()(__left, __right), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_and_epi64(__mask,
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));

		else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
			return __intrin_bitcast<_IntrinType_>(_mm512_maskz_and_epi32(__mask,
				__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right)));

		else
			return _Maskz_assign<arch::ISA::AVX512F, 512, _DesiredType_>()(
				_And<arch::ISA::AVX512F, 512>()(__left, __right), __mask);
	}
};

template <class _DesiredType_>
struct _Maskz_and<arch::ISA::AVX512BW, 512, _DesiredType_> :
	_Maskz_and<arch::ISA::AVX512F, 512, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::AVX512BW, 512, _DesiredType_>()(
			_And<arch::ISA::AVX512BW, 512>()(__left, __right), __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__left,
		_IntrinType_	__right,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) >= 4)
			return _Maskz_and<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right, __mask);
		else
			return _Maskz_assign<arch::ISA::AVX512BW, 512, _DesiredType_>()(
				_And<arch::ISA::AVX512BW, 512>()(__left, __right), __mask);
	}
};


template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Maskz_and<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Maskz_and<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Maskz_and<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Maskz_and<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Maskz_and<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Maskz_and<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
