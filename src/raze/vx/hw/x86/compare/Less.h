#pragma once 

#include <src/raze/vx/hw/x86/mask/ToMask.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_Type_>
struct _Less;

template <class _Type_>
struct _Less<arch::ISA::SSE2, 128, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
            requires(__is_intrin_type_v<_IntrinType_>)
    {
        if constexpr (__is_epi64_v<_Type_>) {
            const auto __difference64 = _mm_sub_epi64(__as<__m128i>(__left), __as<__m128i>(__right));

            const auto __xor_mask = _mm_xor_si128(__as<__m128i>(__left), __as<__m128i>(__right));
            const auto __left_andnot_right = _mm_andnot_si128(__as<__m128i>(__right), __as<__m128i>(__left));
            const auto __difference_andnot_xor = _mm_andnot_si128(__xor_mask, __difference64);

            const auto __combined_mask = _mm_or_si128(__left_andnot_right, __difference_andnot_xor);

            const auto __sign_bits32 = _mm_srai_epi32(__combined_mask, 31);
            const auto __sign_bits64 = _mm_shuffle_epi32(__sign_bits32, 0xF5);

            return __as<_IntrinType_>(__sign_bits64);
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            const auto __32bit_sign = _mm_set1_epi32(0x80000000);

            const auto __signed_32bit_left = _mm_xor_si128(__as<__m128i>(__left), __32bit_sign);
            const auto __signed_32bit_right = _mm_xor_si128(__as<__m128i>(__right), __32bit_sign);
        
            const auto __equal = _mm_cmpeq_epi32(__as<__m128i>(__left),
                __as<__m128i>(__right));
            const auto __bigger = _mm_cmplt_epi32(__signed_32bit_left, __signed_32bit_right);

            const auto __shuffled_bigger = _mm_shuffle_epi32(__bigger, 0xA0);
            const auto __equal_bigger = _mm_and_si128(__equal, __shuffled_bigger);

            const auto __result = _mm_shuffle_epi32(_mm_or_si128(__bigger, __equal_bigger), 0xF5);
            return __as<_IntrinType_>(__result);
        }
        else if constexpr (__is_epi32_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmplt_epi32(
                __as<__m128i>(__left),
                __as<__m128i>(__right)));
        }
        else if constexpr (__is_epu32_v<_Type_>) {
            const auto __sign        = _mm_set1_epi32(0x80000000);

            const auto __signed_left  = _mm_xor_si128(__as<__m128i>(__left), __sign);
            const auto __signed_right = _mm_xor_si128(__as<__m128i>(__right), __sign);
        
            return __as<_IntrinType_>(_mm_cmplt_epi32(__signed_left, __signed_right));
        }
        else if constexpr (__is_epi16_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmplt_epi16(
                __as<__m128i>(__left), 
                __as<__m128i>(__right)));
        }
        else if constexpr (__is_epu16_v<_Type_>) {
            const auto __substracted = _mm_subs_epu16(
                __as<__m128i>(__right),
                __as<__m128i>(__left));

            return _Not<arch::ISA::SSE2, 128>()(__as<_IntrinType_>(
                _mm_cmpeq_epi16(__substracted, _mm_setzero_si128())));
        }
        else if constexpr (__is_epi8_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmplt_epi8(
                __as<__m128i>(__left), __as<__m128i>(__right)));
        }
        else if constexpr (__is_epu8_v<_Type_>) {
            const auto __substracted = _mm_subs_epu8(
                __as<__m128i>(__right), 
                __as<__m128i>(__left));

            return _Not<arch::ISA::SSE2, 128>()(__as<_IntrinType_>(
                _mm_cmpeq_epi8(__substracted, _mm_setzero_si128())));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmplt_ps(
                __as<__m128>(__left), __as<__m128>(__right)));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmplt_pd(
                __as<__m128d>(__left), __as<__m128d>(__right)));
        }
    }
};

template <class _Type_>
struct _Less<arch::ISA::SSE42, 128, _Type_>:
    _Less<arch::ISA::SSE41, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpgt_epi64(
                __as<__m128i>(__right), __as<__m128i>(__left)));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            const auto __sign_64bit = _mm_set1_epi64x(0x8000000000000000);

            const auto __left_signed = _mm_xor_si128(__as<__m128i>(__left), __sign_64bit);
            const auto __right_signed = _mm_xor_si128(__as<__m128i>(__right), __sign_64bit);

            return __as<_IntrinType_>(_mm_cmpgt_epi64(__right_signed, __left_signed));
        }
        else {
            return _Less<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
        }
    }
};

template <class _Type_> 
struct _Less<arch::ISA::AVX, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_pd_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmp_pd(
                __as<__m256d>(__left), __as<__m256d>(__right), _MM_CMPINT_LT));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmp_ps(
                __as<__m256>(__left), __as<__m256>(__right), _MM_CMPINT_LT));
        }
        else {
            const auto __low = _Less<arch::ISA::SSE42, 128, _Type_>()(
                __as<__m128i>(__left), __as<__m128i>(__right));

            const auto __high = _Less<arch::ISA::SSE42, 128, _Type_>()(
                _mm256_extractf128_si256(__as<__m256i>(__left), 1),
                _mm256_extractf128_si256(__as<__m256i>(__right), 1));

            return __as<_IntrinType_>(_mm256_insertf128_si256(
                __as<__m256i>(__low), __high, 1));
        }
    }
};

template <class _Type_>
struct _Less<arch::ISA::AVX2, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_pd_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmp_pd(
                __as<__m256d>(__left), __as<__m256d>(__right), _MM_CMPINT_LT));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmp_ps(
                __as<__m256>(__left), __as<__m256>(__right), _MM_CMPINT_LT));
        }
        else if constexpr (__is_epi64_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmpgt_epi64(
                __as<__m256i>(__right), __as<__m256i>(__left)));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            const auto __sign_64bit = _mm256_set1_epi64x(0x8000000000000000);

            const auto __left_signed  = _mm256_xor_si256(__as<__m256i>(__left), __sign_64bit);
            const auto __right_signed = _mm256_xor_si256(__as<__m256i>(__right), __sign_64bit);

            return __as<_IntrinType_>(_mm256_cmpgt_epi64(__right_signed, __left_signed));
        }
        else if constexpr (__is_epi32_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmpgt_epi32(
                __as<__m256i>(__right), __as<__m256i>(__left)));
        }
        else if constexpr (__is_epu32_v<_Type_>) {
            const auto __sign_32bit = _mm256_set1_epi32(0x80000000);

            const auto __left_signed = _mm256_xor_si256(__as<__m256i>(__left), __sign_32bit);
            const auto __right_signed = _mm256_xor_si256(__as<__m256i>(__right), __sign_32bit);

            return __as<_IntrinType_>(_mm256_cmpgt_epi32(__right_signed, __left_signed));
        }
        else if constexpr (__is_epi16_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmpgt_epi16(
                __as<__m256i>(__right), __as<__m256i>(__left)));
        }
        else if constexpr (__is_epu16_v<_Type_>) {
            const auto __sign_16bit = _mm256_set1_epi16(0x8000);

            const auto __left_signed    = _mm256_xor_si256(__as<__m256i>(__left), __sign_16bit);
            const auto __right_signed   = _mm256_xor_si256(__as<__m256i>(__right), __sign_16bit);

            return __as<_IntrinType_>(_mm256_cmpgt_epi16(__right_signed, __left_signed));
        }
        else if constexpr (__is_epi8_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmpgt_epi8(
                __as<__m256i>(__right), __as<__m256i>(__left)));
        }
        else if constexpr (__is_epu8_v<_Type_>) {
            const auto __sign_8bit   = _mm256_set1_epi8(0x80);

            const auto __left_signed    = _mm256_xor_si256(__as<__m256i>(__left), __sign_8bit);
            const auto __right_signed   = _mm256_xor_si256(__as<__m256i>(__right), __sign_8bit);

            return __as<_IntrinType_>(_mm256_cmpgt_epi8(__right_signed, __left_signed));
        }
    }
};

template <class _Type_>
struct _Less<arch::ISA::AVX512F, 512, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>) {
            return _mm512_cmplt_epi64_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            return _mm512_cmplt_epu64_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));
        }
        else if constexpr (__is_epi32_v<_Type_>) {
            return _mm512_cmplt_epi32_mask(
                __as<__m512i>(__left), 
                __as<__m512i>(__right));
        }
        else if constexpr (__is_epu32_v<_Type_>) {
            return _mm512_cmplt_epu32_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return _mm512_cmplt_ps_mask(
                __as<__m512>(__left), 
                __as<__m512>(__right));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            return _mm512_cmplt_pd_mask(
                __as<__m512d>(__left),
                __as<__m512d>(__right));
        }
        else {
            const auto __compared_low = _Less<arch::ISA::AVX2, 256, _Type_>()(
                __as<__m256i>(__left), __as<__m256i>(__right));

            const auto __compared_high = _Less<arch::ISA::AVX2, 256, _Type_>()(
                _mm512_extracti64x4_epi64(__as<__m512i>(__left), 1),
                _mm512_extracti64x4_epi64(__as<__m512i>(__right), 1));

            return __as<_IntrinType_>(_mm512_inserti64x4(
                __as<__m512i>(__compared_low), __compared_high, 1));
        }
    }
};

template <class _Type_>
struct _Less<arch::ISA::AVX512BW, 512, _Type_>: 
    _Less<arch::ISA::AVX512F, 512, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi16_v<_Type_>)
            return _mm512_cmplt_epi16_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm512_cmplt_epu16_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));

        else if constexpr (__is_epi8_v<_Type_>)
            return _mm512_cmplt_epi8_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm512_cmplt_epu8_mask(
                __as<__m512i>(__left),
                __as<__m512i>(__right));

        else
            return _Less<arch::ISA::AVX512F, 512, _Type_>()(__left, __right);
    }
};

template <class _Type_>
struct _Less<arch::ISA::AVX512VLF, 256, _Type_>:
    _Less<arch::ISA::AVX2, 256, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>)
            return _mm256_cmplt_epi64_mask(
                __as<__m256i>(__left),
                __as<__m256i>(__right));
        
        else if constexpr (__is_epu64_v<_Type_>)
            return _mm256_cmplt_epu64_mask(
                __as<__m256i>(__left), 
                __as<__m256i>(__right));
        
        else if constexpr (__is_epi32_v<_Type_>)
            return _mm256_cmplt_epi32_mask(
                __as<__m256i>(__left), 
                __as<__m256i>(__right));
        
        else if constexpr (__is_epu32_v<_Type_>)
            return _mm256_cmplt_epu32_mask(
                __as<__m256i>(__left), 
                __as<__m256i>(__right));
        
        else if constexpr (__is_ps_v<_Type_>)
            return _mm256_cmp_ps_mask(
                __as<__m256>(__left), 
                __as<__m256>(__right), _CMP_LT_OQ);
        
        else if constexpr (__is_pd_v<_Type_>)
            return _mm256_cmp_pd_mask(
                __as<__m256d>(__left), 
                __as<__m256d>(__right), _CMP_LT_OQ);
        
        else
            return _Less<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
    }
};

template <class _Type_> 
struct _Less<arch::ISA::AVX512VLBW, 256, _Type_>:
    _Less<arch::ISA::AVX512VLF, 256, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi16_v<_Type_>)
            return _mm256_cmplt_epi16_mask(
                __as<__m256i>(__left), 
                __as<__m256i>(__right));
        
        else if constexpr (__is_epu16_v<_Type_>)
            return _mm256_cmplt_epu16_mask(
                __as<__m256i>(__left),
                __as<__m256i>(__right));
        
        else if constexpr (__is_epi8_v<_Type_>)
            return _mm256_cmplt_epi8_mask(
                __as<__m256i>(__left),
                __as<__m256i>(__right));
        
        else if constexpr (__is_epu8_v<_Type_>)
            return _mm256_cmplt_epu8_mask(
                __as<__m256i>(__left), 
                __as<__m256i>(__right));
        
        else
            return _Less<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
    }
};


template <class _Type_>
struct _Less<arch::ISA::AVX512VLF, 128, _Type_> :
    _Less<arch::ISA::AVX2, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi64_v<_Type_>)
            return _mm_cmplt_epi64_mask(
                __as<__m128i>(__left), 
                __as<__m128i>(__right));

        else if constexpr (__is_epu64_v<_Type_>)
            return _mm_cmplt_epu64_mask(
                __as<__m128i>(__left), 
                __as<__m128i>(__right));

        else if constexpr (__is_epi32_v<_Type_>)
            return _mm_cmplt_epi32_mask(
                __as<__m128i>(__left),
                __as<__m128i>(__right));

        else if constexpr (__is_epu32_v<_Type_>)
            return _mm_cmplt_epu32_mask(
                __as<__m128i>(__left),
                __as<__m128i>(__right));

        else if constexpr (__is_ps_v<_Type_>)
            return _mm_cmp_ps_mask(
                __as<__m128>(__left),
                __as<__m128>(__right), _CMP_LT_OQ);

        else if constexpr (__is_pd_v<_Type_>)
            return _mm_cmp_pd_mask(
                __as<__m128d>(__left),
                __as<__m128d>(__right), _CMP_LT_OQ);

        else
            return _Less<arch::ISA::AVX2, 128, _Type_>()(__left, __right);
    }
};

template <class _Type_>
struct _Less<arch::ISA::AVX512VLBW, 128, _Type_> :
    _Less<arch::ISA::AVX512VLF, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        if constexpr (__is_epi16_v<_Type_>)
            return _mm_cmplt_epi16_mask(
                __as<__m128i>(__left), 
                __as<__m128i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm_cmplt_epu16_mask(
                __as<__m128i>(__left), 
                __as<__m128i>(__right));

        else if constexpr (__is_epi8_v<_Type_>)
            return _mm_cmplt_epi8_mask(
                __as<__m128i>(__left),
                __as<__m128i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm_cmplt_epu8_mask(
                __as<__m128i>(__left), 
                __as<__m128i>(__right));

        else
            return _Less<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
    }
};

template <class _Type_> struct _Less<arch::ISA::SSE3, 128, _Type_> : _Less<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::SSSE3, 128, _Type_> : _Less<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::SSE41, 128, _Type_> : _Less<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX, 128, _Type_> : _Less<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX2, 128, _Type_> : _Less<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::FMA3, 128, _Type_> : _Less<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX2FMA3, 128, _Type_> : _Less<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Less<arch::ISA::AVX512DQ, 512, _Type_> : _Less<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512BWDQ, 512, _Type_> : _Less<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI, 512, _Type_> : _Less<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI2, 512, _Type_> : _Less<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Less<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Less<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Less<arch::ISA::FMA3, 256, _Type_> : _Less<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX2FMA3, 256, _Type_> : _Less<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VLDQ, 256, _Type_> : _Less<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Less<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Less<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Less<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Less<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Less<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Less<arch::ISA::AVX512VLDQ, 128, _Type_> : _Less<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Less<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Less<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Less<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Less<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Less<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Less<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
