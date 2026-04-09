#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/shuffle/MaskExpand.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Blend;

template <class _DesiredType_>
struct _Blend<arch::ISA::SSE2, 128, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return __intrin_bitcast<_IntrinType_>(_mm_or_si128(
			_mm_and_si128(__intrin_bitcast<__m128i>(__mask),
                __intrin_bitcast<__m128i>(__first)),
			_mm_andnot_si128(__intrin_bitcast<__m128i>(__mask),
                __intrin_bitcast<__m128i>(__second))));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Blend()(__first, __second, _To_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_> struct _Blend<arch::ISA::SSE3, 128, _DesiredType_>: _Blend<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::SSSE3, 128, _DesiredType_>: _Blend<arch::ISA::SSE3, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Blend<arch::ISA::SSE41, 128, _DesiredType_>:
	_Blend<arch::ISA::SSSE3, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) == 8) 
			return __intrin_bitcast<_IntrinType_>(_mm_blendv_pd(
                __intrin_bitcast<__m128d>(__second),
				__intrin_bitcast<__m128d>(__first),
                __intrin_bitcast<__m128d>(__mask)));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return __intrin_bitcast<_IntrinType_>(_mm_blendv_ps(
                __intrin_bitcast<__m128>(__second),
				__intrin_bitcast<__m128>(__first),
                __intrin_bitcast<__m128>(__mask)));

		else
			return __intrin_bitcast<_IntrinType_>(_mm_blendv_epi8(
                __intrin_bitcast<__m128i>(__second),
				__intrin_bitcast<__m128i>(__first),
                __intrin_bitcast<__m128i>(__mask)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Blend()(__first, __second, _To_vector<arch::ISA::SSE41, 128, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_> struct _Blend<arch::ISA::SSE42, 128, _DesiredType_> : _Blend<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX, 128, _DesiredType_> : _Blend<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX2, 128, _DesiredType_> : _Blend<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _Blend<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::FMA3, 128, _DesiredType_> : _Blend<arch::ISA::AVX, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>:
	_Blend<arch::ISA::AVX2, 128, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return __intrin_bitcast<_IntrinType_>(_mm_ternarylogic_epi32(
            __intrin_bitcast<__m128i>(__mask),
	        __intrin_bitcast<__m128i>(__first),
            __intrin_bitcast<__m128i>(__second), 0xCA));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) == 8)
			return __intrin_bitcast<_IntrinType_>(_mm_mask_blend_epi64(
                __mask,	__intrin_bitcast<__m128i>(__second),
                __intrin_bitcast<__m128i>(__first)));

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm_mask_blend_epi32(
                __mask, __intrin_bitcast<__m128i>(__second),
                __intrin_bitcast<__m128i>(__first)));

        else
            return (*this)(__first, __second, _To_vector<arch::ISA::AVX512VLF,
                256, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_> 
struct _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
    _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__first, __second, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
    {
        if constexpr (sizeof(_DesiredType_) == 2)
            return __intrin_bitcast<_IntrinType_>(_mm_mask_blend_epi16(
                __mask, __intrin_bitcast<__m128i>(__second),
                __intrin_bitcast<__m128i>(__first)));

        else if constexpr (sizeof(_DesiredType_) == 1)
            return __intrin_bitcast<_IntrinType_>(_mm_mask_blend_epi8(
                __mask, __intrin_bitcast<__m128i>(__second),
                __intrin_bitcast<__m128i>(__first)));

        else 
            return _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__first, __second, __mask);
    }
};

template <class _DesiredType_>
struct _Blend<arch::ISA::AVX, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) == 8)
			return __intrin_bitcast<_IntrinType_>(_mm256_blendv_pd(
				__intrin_bitcast<__m256d>(__second),
				__intrin_bitcast<__m256d>(__first),
				__intrin_bitcast<__m256d>(__mask)));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return __intrin_bitcast<_IntrinType_>(_mm256_blendv_ps(
				__intrin_bitcast<__m256>(__second),
				__intrin_bitcast<__m256>(__first),
				__intrin_bitcast<__m256>(__mask)));

		else
			return __intrin_bitcast<_IntrinType_>(_mm256_or_ps(
				_mm256_and_ps(__intrin_bitcast<__m256>(__mask),
					__intrin_bitcast<__m256>(__first)),
				_mm256_andnot_ps(__intrin_bitcast<__m256>(__mask),
					__intrin_bitcast<__m256>(__second))));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return (*this)(__first, __second, _To_vector<arch::ISA::AVX, 256, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_> struct _Blend<arch::ISA::FMA3, 256, _DesiredType_> : _Blend<arch::ISA::AVX, 256, _DesiredType_> {};

template <class _DesiredType_>
struct _Blend<arch::ISA::AVX2, 256, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_DesiredType_) == 8)
			return __intrin_bitcast<_IntrinType_>(_mm256_blendv_pd(
                __intrin_bitcast<__m256d>(__second),
				__intrin_bitcast<__m256d>(__first),
                __intrin_bitcast<__m256d>(__mask)));

		else if constexpr (sizeof(_DesiredType_) == 4)
			return __intrin_bitcast<_IntrinType_>(_mm256_blendv_ps(
                __intrin_bitcast<__m256>(__second),
				__intrin_bitcast<__m256>(__first),
                __intrin_bitcast<__m256>(__mask)));

		else
			return __intrin_bitcast<_IntrinType_>(_mm256_blendv_epi8(
                __intrin_bitcast<__m256i>(__second),
				__intrin_bitcast<__m256i>(__first),
                __intrin_bitcast<__m256i>(__mask)));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
		return _Blend()(__first, __second, _To_vector<arch::ISA::AVX2,
            256, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_> struct _Blend<arch::ISA::AVX2FMA3, 256, _DesiredType_>: _Blend<arch::ISA::AVX2, 256, _DesiredType_>  {};

template <class _DesiredType_> 
struct _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>: 
    _Blend<arch::ISA::AVX2, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
	    return __intrin_bitcast<_IntrinType_>(_mm256_ternarylogic_epi32(
            __intrin_bitcast<__m256i>(__mask),
	        __intrin_bitcast<__m256i>(__first),
            __intrin_bitcast<__m256i>(__second), 0xCA));	
    }

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm256_mask_blend_epi64(
                __mask, __intrin_bitcast<__m256i>(__second),
                __intrin_bitcast<__m256i>(__first)));

        if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm256_mask_blend_epi32(
                __mask, __intrin_bitcast<__m256i>(__second),
                __intrin_bitcast<__m256i>(__first)));

        else
            return (*this)(__first, __second, _To_vector<arch::ISA::AVX512VLF, 
                256, _IntrinType_, _DesiredType_>()(__mask));
    }   
};

template <class _DesiredType_> 
struct _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_>: 
    _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
	    return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__first, __second, __mask);
    }

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        if constexpr (sizeof(_DesiredType_) == 2)
            return __intrin_bitcast<_IntrinType_>(_mm256_mask_blend_epi16(
                __mask, __intrin_bitcast<__m256i>(__second),
                __intrin_bitcast<__m256i>(__first)));

        else if constexpr (sizeof(_DesiredType_) == 1)
            return __intrin_bitcast<_IntrinType_>(_mm256_mask_blend_epi8(
                __mask, __intrin_bitcast<__m256i>(__second),
                __intrin_bitcast<__m256i>(__first)));

        else 
            return _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__first, __second, __mask);
    }
};

template <class _DesiredType_>
struct _Blend<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
        return __intrin_bitcast<_IntrinType_>(_mm512_ternarylogic_epi32(
            __intrin_bitcast<__m512i>(__mask),
	        __intrin_bitcast<__m512i>(__first),
            __intrin_bitcast<__m512i>(__second), 0xCA));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
        if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm512_mask_blend_epi64(
                __mask, __intrin_bitcast<__m512i>(__second),
                __intrin_bitcast<__m512i>(__first)));

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm512_mask_blend_epi32(
                __mask, __intrin_bitcast<__m512i>(__second),
				__intrin_bitcast<__m512i>(__first)));

        else
		    return _Blend()(__first, __second, _To_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>()(__mask));
	}
};

template <class _DesiredType_>
struct _Blend<arch::ISA::AVX512BW, 512, _DesiredType_>: 
	_Blend<arch::ISA::AVX512F, 512, _DesiredType_> 
{
	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_>)
	{
		return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(__first, __second, __mask);
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		_IntrinType_	__first,
		_IntrinType_	__second,
		_MaskType_		__mask) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_>)
	{
        if constexpr (sizeof(_DesiredType_) == 2)
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_blend_epi16(
				__mask, __intrin_bitcast<__m512i>(__second),
				__intrin_bitcast<__m512i>(__first)));

		else if constexpr (sizeof(_DesiredType_) == 1)
			return __intrin_bitcast<_IntrinType_>(_mm512_mask_blend_epi8(
				__mask, __intrin_bitcast<__m512i>(__second),
				__intrin_bitcast<__m512i>(__first)));
        else
            return _Blend<arch::ISA::AVX512F, 512, _DesiredType_>()(__first, __second, __mask);
	}
};

template <class _DesiredType_> struct _Blend<arch::ISA::AVX512DQ, 512, _DesiredType_>: _Blend<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512BWDQ, 512, _DesiredType_>: _Blend<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI, 512, _DesiredType_>: _Blend<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI2, 512, _DesiredType_>: _Blend<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_>: _Blend<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_>: _Blend<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VLDQ, 256, _DesiredType_>: _Blend<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_>: _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMIVL, 256, _DesiredType_>: _Blend<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_>: _Blend<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_>: _Blend<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_>: _Blend<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VLDQ, 128, _DesiredType_>: _Blend<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_>: _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMIVL, 128, _DesiredType_>: _Blend<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_>: _Blend<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_>: _Blend<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Blend<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_>: _Blend<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
