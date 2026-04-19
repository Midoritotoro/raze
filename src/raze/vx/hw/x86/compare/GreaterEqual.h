#pragma once 

#include <src/raze/vx/hw/x86/compare/Less.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_Type_>
struct _Greater_equal;

template <class _Type_>
struct _Greater_equal<arch::ISA::SSE2, 128, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_pd_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpge_pd(
                __as<__m128d>(__left), __as<__m128d>(__right)));

        else if constexpr (__is_ps_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpge_ps(
                __as<__m128>(__left), __as<__m128>(__right)));

        else
            return _Not<arch::ISA::SSE2, 128, _Type_>()(_Less<arch::ISA::SSE2, 128, _Type_>()(__left, __right));
    }
};

template <class _Type_> 
struct _Greater_equal<arch::ISA::AVX, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        return _Not<arch::ISA::AVX, 256, _Type_>()(_Less<arch::ISA::AVX, 256, _Type_>()(__left, __right));
    }
};

template <class _Type_>
struct _Greater_equal<arch::ISA::AVX2, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        return _Not<arch::ISA::AVX2, 256, _Type_>()(_Less<arch::ISA::AVX2, 256, _Type_>()(__left, __right));
    }
};

template <class _Type_>
struct _Greater_equal<arch::ISA::AVX512F, 512, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>) {
            return _mm512_cmpge_epi64_mask(__as<__m512i>(__left), __as<__m512i>(__right));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            return _mm512_cmpge_epu64_mask(__as<__m512i>(__left), __as<__m512i>(__right));
        }
        else if constexpr (__is_epi32_v<_Type_>) {
            return _mm512_cmpge_epi32_mask(__as<__m512i>(__left), __as<__m512i>(__right));
        }
        else if constexpr (__is_epu32_v<_Type_>) {
            return _mm512_cmpge_epu32_mask(__as<__m512i>(__left), __as<__m512i>(__right));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            return _mm512_cmp_pd_mask(__as<__m512d>(__left), __as<__m512d>(__right), _CMP_GE_OQ);
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return _mm512_cmp_ps_mask(__as<__m512>(__left), __as<__m512>(__right), _CMP_GE_OQ);
        }
        else {
            const auto __compared_low = _Greater_equal<arch::ISA::AVX2, 256, _Type_>()(
                __as<__m256i>(__left), __as<__m256i>(__right));

            const auto __compared_high = _Greater_equal<arch::ISA::AVX2, 256, _Type_>()(
                _mm512_extracti64x4_epi64(__as<__m512i>(__left), 1),
                _mm512_extracti64x4_epi64(__as<__m512i>(__right), 1));

            return __as<_IntrinType_>(_mm512_inserti64x4(
                __as<__m512i>(__compared_low), __compared_high, 1));
        }
    }
};

template <class _Type_> 
struct _Greater_equal<arch::ISA::AVX512BW, 512, _Type_>:
    _Greater_equal<arch::ISA::AVX512F, 512, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi8_v<_Type_>)
            return _mm512_cmpge_epi8_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm512_cmpge_epu8_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else if constexpr (__is_epi16_v<_Type_>)
            return _mm512_cmpge_epi16_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm512_cmpge_epu16_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else
            return _Greater_equal<arch::ISA::AVX512F, 512, _Type_>()(__left, __right);
    }
};

template <class _Type_> 
struct _Greater_equal<arch::ISA::AVX512VLF, 256, _Type_>:
    _Greater_equal<arch::ISA::AVX2, 256, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>) {
            return _mm256_cmpge_epi64_mask(__as<__m256i>(__left), __as<__m256i>(__right));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            return _mm256_cmpge_epu64_mask(__as<__m256i>(__left), __as<__m256i>(__right));
        }
        else if constexpr (__is_epi32_v<_Type_>) {
            return _mm256_cmpge_epi32_mask(__as<__m256i>(__left), __as<__m256i>(__right));
        }
        else if constexpr (__is_epu32_v<_Type_>) {
            return _mm256_cmpge_epu32_mask(__as<__m256i>(__left), __as<__m256i>(__right));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            return _mm256_cmp_pd_mask(__as<__m256d>(__left), __as<__m256d>(__right), _CMP_GE_OQ);
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return _mm256_cmp_ps_mask(__as<__m256>(__left), __as<__m256>(__right), _CMP_GE_OQ);
        }
        else {
            return _Greater_equal<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
        }
    }
};

template <class _Type_> 
struct _Greater_equal<arch::ISA::AVX512VLBW, 256, _Type_>:
    _Greater_equal<arch::ISA::AVX512VLF, 256, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi8_v<_Type_>)
            return _mm256_cmpge_epi8_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm256_cmpge_epu8_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epi16_v<_Type_>)
            return _mm256_cmpge_epi16_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm256_cmpge_epu16_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else
            return _Greater_equal<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
    }
};

template <class _Type_>
struct _Greater_equal<arch::ISA::AVX512VLF, 128, _Type_> :
    _Greater_equal<arch::ISA::AVX2, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>) {
            return _mm_cmpge_epi64_mask(__as<__m128i>(__left), __as<__m128i>(__right));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            return _mm_cmpge_epu64_mask(__as<__m128i>(__left), __as<__m128i>(__right));
        }
        else if constexpr (__is_epi32_v<_Type_>) {
            return _mm_cmpge_epi32_mask(__as<__m128i>(__left), __as<__m128i>(__right));
        }
        else if constexpr (__is_epu32_v<_Type_>) {
            return _mm_cmpge_epu32_mask(__as<__m128i>(__left), __as<__m128i>(__right));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            return _mm_cmp_pd_mask(__as<__m128d>(__left), __as<__m128d>(__right), _CMP_GE_OQ);
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return _mm_cmp_ps_mask(__as<__m128>(__left), __as<__m128>(__right), _CMP_GE_OQ);
        }
        else {
            return _Greater_equal<arch::ISA::AVX2, 128, _Type_>()(__left, __right);
        }
    }
};

template <class _Type_>
struct _Greater_equal<arch::ISA::AVX512VLBW, 128, _Type_> :
    _Greater_equal<arch::ISA::AVX512VLF, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi8_v<_Type_>)
            return _mm_cmpge_epi8_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm_cmpge_epu8_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epi16_v<_Type_>)
            return _mm_cmpge_epi16_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm_cmpge_epu16_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else
            return _Greater_equal<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right);
    }
};

template <class _Type_> struct _Greater_equal<arch::ISA::SSE3, 128, _Type_> : _Greater_equal<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::SSSE3, 128, _Type_> : _Greater_equal<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::SSE41, 128, _Type_> : _Greater_equal<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::SSE42, 128, _Type_> : _Greater_equal<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX, 128, _Type_> : _Greater_equal<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX2, 128, _Type_> : _Greater_equal<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::FMA3, 128, _Type_> : _Greater_equal<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX2FMA3, 128, _Type_> : _Greater_equal<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Greater_equal<arch::ISA::AVX512DQ, 512, _Type_> : _Greater_equal<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512BWDQ, 512, _Type_> : _Greater_equal<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI, 512, _Type_> : _Greater_equal<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI2, 512, _Type_> : _Greater_equal<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Greater_equal<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Greater_equal<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Greater_equal<arch::ISA::FMA3, 256, _Type_> : _Greater_equal<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX2FMA3, 256, _Type_> : _Greater_equal<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VLDQ, 256, _Type_> : _Greater_equal<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Greater_equal<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Greater_equal<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Greater_equal<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Greater_equal<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Greater_equal<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VLDQ, 128, _Type_> : _Greater_equal<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Greater_equal<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Greater_equal<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Greater_equal<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Greater_equal<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Greater_equal<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Greater_equal<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
