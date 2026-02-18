#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/datapar/bitwise/ToMask.h>
#include <src/raze/datapar/bitwise/BitNot.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_DesiredType_>
struct _Simd_less;

template <class _DesiredType_>
struct _Simd_less<arch::ISA::SSE2, 128, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            const auto __difference64 = _mm_sub_epi64(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));

            const auto __xor_mask = _mm_xor_si128(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));
            const auto __left_andnot_right = _mm_andnot_si128(__intrin_bitcast<__m128i>(__right), __intrin_bitcast<__m128i>(__left));
            const auto __difference_andnot_xor = _mm_andnot_si128(__xor_mask, __difference64);

            const auto __combined_mask = _mm_or_si128(__left_andnot_right, __difference_andnot_xor);

            const auto __sign_bits32 = _mm_srai_epi32(__combined_mask, 31);
            const auto __sign_bits64 = _mm_shuffle_epi32(__sign_bits32, 0xF5);

            return __intrin_bitcast<_IntrinType_>(__sign_bits64);
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            const auto __32bit_sign       = _mm_set1_epi32(0x80000000);

            const auto __signed_32bit_left  = _mm_xor_si128(__intrin_bitcast<__m128i>(__left), __32bit_sign);
            const auto __signed_32bit_right = _mm_xor_si128(__intrin_bitcast<__m128i>(__right), __32bit_sign);
        
            const auto __equal   = _mm_cmpeq_epi32(__intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));
            const auto __bigger  = _mm_cmplt_epi32(__signed_32bit_left, __signed_32bit_right);

            const auto __shuffled_bigger  = _mm_shuffle_epi32(__bigger, 0xA0);
            const auto __equal_bigger     = _mm_and_si128(__equal, __shuffled_bigger);

            const auto __result = _mm_shuffle_epi32(_mm_or_si128(__bigger, __equal_bigger), 0xF5);
            return __intrin_bitcast<_IntrinType_>(__result);
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmplt_epi32(
                __intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            const auto __sign        = _mm_set1_epi32(0x80000000);

            const auto __signed_left  = _mm_xor_si128(__intrin_bitcast<__m128i>(__left), __sign);
            const auto __signed_right = _mm_xor_si128(__intrin_bitcast<__m128i>(__right), __sign);
        
            return __intrin_bitcast<_IntrinType_>(_mm_cmplt_epi32(__signed_left, __signed_right));
        }
        else if constexpr (__is_epi16_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmplt_epi16(
                __intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
        }
        else if constexpr (__is_epu16_v<_DesiredType_>) {
            const auto __substracted = _mm_subs_epu16(__intrin_bitcast<__m128i>(__right), __intrin_bitcast<__m128i>(__left));
            return _Simd_bit_not<arch::ISA::SSE2, 128>()(__intrin_bitcast<_IntrinType_>(
                _mm_cmpeq_epi16(__substracted, _mm_setzero_si128())));
        }
        else if constexpr (__is_epi8_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmplt_epi8(
                __intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
        }
        else if constexpr (__is_epu8_v<_DesiredType_>) {
            const auto __substracted = _mm_subs_epu8(__intrin_bitcast<__m128i>(__right), __intrin_bitcast<__m128i>(__left));
            return _Simd_bit_not<arch::ISA::SSE2, 128>()(__intrin_bitcast<_IntrinType_>(
                _mm_cmpeq_epi8(__substracted, _mm_setzero_si128())));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmplt_ps(
                __intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__right)));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmplt_pd(
                __intrin_bitcast<__m128d>(__left), __intrin_bitcast<__m128d>(__right)));
        }
    }
};

template <class _DesiredType_>
struct _Simd_less<arch::ISA::SSE42, 128, _DesiredType_>:
    _Simd_less<arch::ISA::SSE41, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi64(
                __intrin_bitcast<__m128i>(__right), __intrin_bitcast<__m128i>(__left)));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            const auto __sign_64bit = _mm_set1_epi64x(0x8000000000000000);

            const auto __left_signed = _mm_xor_si128(__intrin_bitcast<__m128i>(__left), __sign_64bit);
            const auto __right_signed = _mm_xor_si128(__intrin_bitcast<__m128i>(__right), __sign_64bit);

            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi64(__right_signed, __left_signed));
        }
        else {
            return _Simd_less<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right);
        }
    }
};

template <class _DesiredType_>
struct _Simd_less<arch::ISA::AVX2, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm256_cmp_pd(
                __intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right), _MM_CMPINT_LT));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm256_cmp_ps(
                __intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right), _MM_CMPINT_LT));
        }
        else if constexpr (__is_epi64_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi64(
                __intrin_bitcast<__m256i>(__right), __intrin_bitcast<__m256i>(__left)));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            const auto __sign_64bit = _mm256_set1_epi64x(0x8000000000000000);

            const auto __left_signed  = _mm256_xor_si256(__intrin_bitcast<__m256i>(__left), __sign_64bit);
            const auto __right_signed = _mm256_xor_si256(__intrin_bitcast<__m256i>(__right), __sign_64bit);

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi64(__right_signed, __left_signed));
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi32(
                __intrin_bitcast<__m256i>(__right), __intrin_bitcast<__m256i>(__left)));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            const auto __sign_32bit = _mm256_set1_epi32(0x80000000);

            const auto __left_signed = _mm256_xor_si256(__intrin_bitcast<__m256i>(__left), __sign_32bit);
            const auto __right_signed = _mm256_xor_si256(__intrin_bitcast<__m256i>(__right), __sign_32bit);

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi32(__right_signed, __left_signed));
        }
        else if constexpr (__is_epi16_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi16(
                __intrin_bitcast<__m256i>(__right), __intrin_bitcast<__m256i>(__left)));
        }
        else if constexpr (__is_epu16_v<_DesiredType_>) {
            const auto __sign_16bit = _mm256_set1_epi16(0x8000);

            const auto __left_signed    = _mm256_xor_si256(__intrin_bitcast<__m256i>(__left), __sign_16bit);
            const auto __right_signed   = _mm256_xor_si256(__intrin_bitcast<__m256i>(__right), __sign_16bit);

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi16(__right_signed, __left_signed));
        }
        else if constexpr (__is_epi8_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi8(
                __intrin_bitcast<__m256i>(__right), __intrin_bitcast<__m256i>(__left)));
        }
        else if constexpr (__is_epu8_v<_DesiredType_>) {
            const auto __sign_8bit   = _mm256_set1_epi8(0x80);

            const auto __left_signed    = _mm256_xor_si256(__intrin_bitcast<__m256i>(__left), __sign_8bit);
            const auto __right_signed   = _mm256_xor_si256(__intrin_bitcast<__m256i>(__right), __sign_8bit);

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi8(__right_signed, __left_signed));
        }
    }
};

template <class _DesiredType_>
struct _Simd_less<arch::ISA::AVX512F, 512, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return _mm512_cmplt_epi64_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            return _mm512_cmplt_epu64_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return _mm512_cmplt_epi32_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            return _mm512_cmplt_epu32_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm512_cmplt_ps_mask(__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm512_cmplt_pd_mask(__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right));
        }
        else {
            const auto __compared_low128 = _Simd_less<arch::ISA::SSE42, 128, _DesiredType_>()(
                __intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));

            const auto __compared2_low128 = _Simd_less<arch::ISA::SSE42, 128, _DesiredType_>()(
                _mm256_extractf128_si256(__intrin_bitcast<__m256i>(__left), 1), _mm256_extractf128_si256(__intrin_bitcast<__m256i>(__right), 1));

            const auto __compared_high128 = _Simd_less<arch::ISA::SSE42, 128, _DesiredType_>()(
                _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__left), 2), _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__right), 2));

            const auto __compared2_high128 = _Simd_less<arch::ISA::SSE42, 128, _DesiredType_>()(
                _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__left), 3), _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__right), 3));

            auto __result = __intrin_bitcast<__m512i>(__compared_low128);

            __result = _mm512_inserti32x4(__result, __compared2_low128, 1);
            __result = _mm512_inserti32x4(__result, __compared_high128, 2);

            return _Simd_to_mask<arch::ISA::AVX512F, 512, _DesiredType_>()(_mm512_inserti32x4(__result, __compared2_high128, 3));
        }
    }
};

template <class _DesiredType_>
struct _Simd_less<arch::ISA::AVX512BW, 512, _DesiredType_>: 
    _Simd_less<arch::ISA::AVX512F, 512, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>)
            return _mm512_cmplt_epi64_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));

        else if constexpr (__is_epu64_v<_DesiredType_>)
            return _mm512_cmplt_epu64_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));

        else if constexpr (__is_epi32_v<_DesiredType_>)
            return _mm512_cmplt_epi32_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        
        else if constexpr (__is_epu32_v<_DesiredType_>)
            return _mm512_cmplt_epu32_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        
        else if constexpr (__is_epi16_v<_DesiredType_>)
            return _mm512_cmplt_epi16_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        
        else if constexpr (__is_epu16_v<_DesiredType_>)
            return _mm512_cmplt_epu16_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        
        else if constexpr (__is_epi8_v<_DesiredType_>)
            return _mm512_cmplt_epi8_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        
        else if constexpr (__is_epu8_v<_DesiredType_>)
            return _mm512_cmplt_epu8_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        
        else if constexpr (__is_ps_v<_DesiredType_>)
            return _mm512_cmplt_ps_mask(__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right));
        
        else if constexpr (__is_pd_v<_DesiredType_>)
            return _mm512_cmplt_pd_mask(__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right));
    }
};

template <class _DesiredType_>
struct _Simd_less<arch::ISA::AVX512VLF, 256, _DesiredType_>:
    _Simd_less<arch::ISA::AVX2, 256, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>)
            return _mm256_cmplt_epi64_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_epu64_v<_DesiredType_>)
            return _mm256_cmplt_epu64_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_epi32_v<_DesiredType_>)
            return _mm256_cmplt_epi32_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_epu32_v<_DesiredType_>)
            return _mm256_cmplt_epu32_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_ps_v<_DesiredType_>)
            return _mm256_cmp_ps_mask(__intrin_bitcast<__m256>(__left), __intrin_bitcast<__m256>(__right), _CMP_LT_OQ);
        
        else if constexpr (__is_pd_v<_DesiredType_>)
            return _mm256_cmp_pd_mask(__intrin_bitcast<__m256d>(__left), __intrin_bitcast<__m256d>(__right), _CMP_LT_OQ);
        
        else
            return _Simd_less<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right);
    }
};

template <class _DesiredType_> 
struct _Simd_less<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
    _Simd_less<arch::ISA::AVX512VLF, 256, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi16_v<_DesiredType_>)
            return _mm256_cmplt_epi16_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_epu16_v<_DesiredType_>)
            return _mm256_cmplt_epu16_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_epi8_v<_DesiredType_>)
            return _mm256_cmplt_epi8_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else if constexpr (__is_epu8_v<_DesiredType_>)
            return _mm256_cmplt_epu8_mask(__intrin_bitcast<__m256i>(__left), __intrin_bitcast<__m256i>(__right));
        
        else
            return _Simd_less<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__left, __right);
    }
};




template <class _DesiredType_> struct _Simd_less<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_less<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_less<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_less<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_less<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_less<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_less<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_less<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_less<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_less<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_less<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_less<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_less<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_less<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
