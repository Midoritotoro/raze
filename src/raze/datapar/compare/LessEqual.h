#pragma once 

#include <src/raze/datapar/bitwise/ToMask.h>
#include <src/raze/datapar/compare/Greater.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_DesiredType_>
struct _Less_equal;

template <class _DesiredType_>
struct _Less_equal<arch::ISA::SSE2, 128, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_DesiredType_>)
            return __intrin_bitcast<_IntrinType_>(_mm_cmple_pd(
                __intrin_bitcast<__m128d>(__left), 
                __intrin_bitcast<__m128d>(__right)));

        else if constexpr (__is_ps_v<_DesiredType_>)
            return __intrin_bitcast<_IntrinType_>(_mm_cmple_ps(
                __intrin_bitcast<__m128>(__left), 
                __intrin_bitcast<__m128>(__right)));

        else
            return _Not<arch::ISA::SSE2, 128>()(_Greater<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right));
    }
};

template <class _DesiredType_> 
struct _Less_equal<arch::ISA::AVX, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Not<arch::ISA::AVX, 256>()(
            _Greater<arch::ISA::AVX, 256, _DesiredType_>()(__left, __right));
    }
};

template <class _DesiredType_>
struct _Less_equal<arch::ISA::AVX2, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Not<arch::ISA::AVX2, 256>()(
            _Greater<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right));
    }
};

template <class _DesiredType_>
struct _Less_equal<arch::ISA::AVX512F, 512, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return _mm512_cmple_epi64_mask(
                __intrin_bitcast<__m512i>(__left), 
                __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            return _mm512_cmple_epu64_mask(
                __intrin_bitcast<__m512i>(__left), 
                __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return _mm512_cmple_epi32_mask(
                __intrin_bitcast<__m512i>(__left), 
                __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            return _mm512_cmple_epu32_mask(
                __intrin_bitcast<__m512i>(__left), 
                __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm512_cmple_pd_mask(
                __intrin_bitcast<__m512d>(__left),
                __intrin_bitcast<__m512d>(__right));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm512_cmple_ps_mask(
                __intrin_bitcast<__m512>(__left),
                __intrin_bitcast<__m512>(__right));
        }
        else {
            const auto __compared_low = _Less_equal<arch::ISA::AVX2, 256, _DesiredType_>()(
                __intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));

            const auto __compared_high = _Less_equal<arch::ISA::AVX2, 256, _DesiredType_>()(
                _mm512_extracti64x4_epi64(__intrin_bitcast<__m512i>(__left), 1),
                _mm512_extracti64x4_epi64(__intrin_bitcast<__m512i>(__right), 1));

            return __intrin_bitcast<_IntrinType_>(_mm512_inserti64x4(
                __intrin_bitcast<__m512i>(__compared_low), __compared_high, 1));
        }
    }
};

template <class _DesiredType_> 
struct _Less_equal<arch::ISA::AVX512BW, 512, _DesiredType_>:
    _Less_equal<arch::ISA::AVX512F, 512, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi8_v<_DesiredType_>)
            return _mm512_cmple_epi8_mask(
                __intrin_bitcast<__m512i>(__left),
                __intrin_bitcast<__m512i>(__right));

        else if constexpr (__is_epu8_v<_DesiredType_>)
            return _mm512_cmple_epu8_mask(
                __intrin_bitcast<__m512i>(__left), 
                __intrin_bitcast<__m512i>(__right));

        else if constexpr (__is_epi16_v<_DesiredType_>)
            return _mm512_cmple_epi16_mask(
                __intrin_bitcast<__m512i>(__left),
                __intrin_bitcast<__m512i>(__right));

        else if constexpr (__is_epu16_v<_DesiredType_>)
            return _mm512_cmple_epu16_mask(
                __intrin_bitcast<__m512i>(__left), 
                __intrin_bitcast<__m512i>(__right));

        else
            return _Less_equal<arch::ISA::AVX512F, 512, _DesiredType_>()(__left, __right);
    }
};

template <class _DesiredType_> 
struct _Less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_>: 
    _Less_equal<arch::ISA::AVX2, 256, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return _mm256_cmple_epi64_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            return _mm256_cmple_epu64_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return _mm256_cmple_epi32_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            return _mm256_cmple_epu32_mask(
                __intrin_bitcast<__m256i>(__left),
                __intrin_bitcast<__m256i>(__right));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm256_cmp_pd_mask(
                __intrin_bitcast<__m256d>(__left),
                __intrin_bitcast<__m256d>(__right), _CMP_LE_OQ);
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm256_cmp_ps_mask(
                __intrin_bitcast<__m256>(__left), 
                __intrin_bitcast<__m256>(__right), _CMP_LE_OQ);
        }
        else {
            return _Less_equal<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right);
        }
    }
};


template <class _DesiredType_> 
struct _Less_equal<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
    _Less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi8_v<_DesiredType_>)
            return _mm256_cmple_epi8_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));

        else if constexpr (__is_epu8_v<_DesiredType_>)
            return _mm256_cmple_epu8_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));

        else if constexpr (__is_epi16_v<_DesiredType_>)
            return _mm256_cmple_epi16_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));

        else if constexpr (__is_epu16_v<_DesiredType_>)
            return _mm256_cmple_epu16_mask(
                __intrin_bitcast<__m256i>(__left), 
                __intrin_bitcast<__m256i>(__right));
        
        else
            return _Less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right);
    }
};

template <class _DesiredType_>
struct _Less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_> :
    _Less_equal<arch::ISA::AVX2, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return _mm_cmple_epi64_mask(
                __intrin_bitcast<__m128i>(__left),
                __intrin_bitcast<__m128i>(__right));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            return _mm_cmple_epu64_mask(
                __intrin_bitcast<__m128i>(__left),
                __intrin_bitcast<__m128i>(__right));
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return _mm_cmple_epi32_mask(
                __intrin_bitcast<__m128i>(__left), 
                __intrin_bitcast<__m128i>(__right));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            return _mm_cmple_epu32_mask(
                __intrin_bitcast<__m128i>(__left), 
                __intrin_bitcast<__m128i>(__right));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm_cmp_pd_mask(
                __intrin_bitcast<__m128d>(__left),
                __intrin_bitcast<__m128d>(__right), _CMP_LE_OQ);
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm_cmp_ps_mask(
                __intrin_bitcast<__m128>(__left), 
                __intrin_bitcast<__m128>(__right), _CMP_LE_OQ);
        }
        else {
            return _Less_equal<arch::ISA::AVX2, 128, _DesiredType_>()(__left, __right);
        }
    }
};


template <class _DesiredType_>
struct _Less_equal<arch::ISA::AVX512VLBW, 128, _DesiredType_> :
    _Less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi8_v<_DesiredType_>)
            return _mm_cmple_epi8_mask(
                __intrin_bitcast<__m128i>(__left), 
                __intrin_bitcast<__m128i>(__right));

        else if constexpr (__is_epu8_v<_DesiredType_>)
            return _mm_cmple_epu8_mask(
                __intrin_bitcast<__m128i>(__left),
                __intrin_bitcast<__m128i>(__right));

        else if constexpr (__is_epi16_v<_DesiredType_>)
            return _mm_cmple_epi16_mask(
                __intrin_bitcast<__m128i>(__left),
                __intrin_bitcast<__m128i>(__right));

        else if constexpr (__is_epu16_v<_DesiredType_>)
            return _mm_cmple_epu16_mask(
                __intrin_bitcast<__m128i>(__left), 
                __intrin_bitcast<__m128i>(__right));

        else
            return _Less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__left, __right);
    }
};


template <class _DesiredType_> struct _Less_equal<arch::ISA::SSE3, 128, _DesiredType_> : _Less_equal<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::SSSE3, 128, _DesiredType_> : _Less_equal<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::SSE41, 128, _DesiredType_> : _Less_equal<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::SSE42, 128, _DesiredType_> : _Less_equal<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX, 128, _DesiredType_> : _Less_equal<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX2, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::FMA3, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Less_equal<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Less_equal<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Less_equal<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Less_equal<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Less_equal<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Less_equal<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Less_equal<arch::ISA::FMA3, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX2FMA3, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Less_equal<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Less_equal<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Less_equal<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
