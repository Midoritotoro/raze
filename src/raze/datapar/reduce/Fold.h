#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_DesiredType_>
struct _Simd_fold;

template <class _DesiredType_>
struct _Simd_fold<arch::ISA::SSE2, 128, _DesiredType_> {
    template <
        class _IntrinType_,
        class _ReduceBinaryFunction_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_            __vector,
        _ReduceBinaryFunction_  __reduce) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 8) {
            auto __folded = __intrin_bitcast<__m128d>(__vector);

            const auto __shuffled = _mm_shuffle_pd(__folded, __folded, 1);
            __folded = __reduce(__shuffled, __folded);

            if constexpr (__is_pd_v<_DesiredType_>)
                return _mm_cvtsd_f64(__folded);
            else
                return _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__folded));
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            auto __folded = __intrin_bitcast<__m128i>(__vector);

            const auto __shuffled1 = _mm_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled2);

            if constexpr (__is_ps_v<_DesiredType_>)
                return _mm_cvtss_f32(__intrin_bitcast<__m128>(__folded));
            else
                return _mm_cvtsi128_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            auto __folded = __intrin_bitcast<__m128i>(__vector);

            __folded = __reduce(__folded, _mm_srli_si128(__folded, 8));
            __folded = __reduce(__folded, _mm_srli_si128(__folded, 4));

            __folded = __reduce(__folded, _mm_srli_si128(__folded, 2));

            return _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__folded));
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            auto __folded = __intrin_bitcast<__m128i>(__vector);

            __folded = __reduce(__folded, _mm_srli_si128(__folded, 8));
            __folded = __reduce(__folded, _mm_srli_si128(__folded, 4));

            __folded = __reduce(__folded, _mm_srli_si128(__folded, 2));
            __folded = __reduce(__folded, _mm_srli_si128(__folded, 1));

            return _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__folded));
        }
    }
};

template <class _DesiredType_> 
struct _Simd_fold<arch::ISA::SSSE3, 128, _DesiredType_>: 
    _Simd_fold<arch::ISA::SSE3, 128, _DesiredType_> 
{
    template <
        class _IntrinType_,
        class _ReduceBinaryFunction_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_            __vector,
        _ReduceBinaryFunction_  __reduce) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 8) {
            auto __folded = __intrin_bitcast<__m128d>(__vector);

            const auto __shuffled = _mm_shuffle_pd(__folded, __folded, 1);
            __folded = __reduce(__shuffled, __folded);

            if constexpr (__is_pd_v<_DesiredType_>)
                return _mm_cvtsd_f64(__folded);
            else
                return _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__folded));
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            auto __folded = __intrin_bitcast<__m128i>(__vector);

            const auto __shuffled1 = _mm_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled2);

            if constexpr (__is_ps_v<_DesiredType_>)
                return _mm_cvtss_f32(__intrin_bitcast<__m128>(__folded));
            else
                return _mm_cvtsi128_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            const auto __shuffle_words = _mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

            auto __folded = __intrin_bitcast<__m128i>(__vector);
        
            const auto __shuffled1 = _mm_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffle2 = _mm_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffle2);

            const auto __shuffle3 = _mm_shuffle_epi8(__folded, __shuffle_words);
            __folded = __reduce(__folded, __shuffle3);

            return _mm_cvtsi128_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __shuffle_bytes = _mm_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1);
            const auto __shuffle_words = _mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

            auto __folded = __intrin_bitcast<__m128i>(__vector);

            const auto __shuffled1 = _mm_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm_shuffle_epi8(__folded, __shuffle_words);
            __folded = __reduce(__folded, __shuffled3);

            const auto __shuffled4 = _mm_shuffle_epi8(__folded, __shuffle_bytes);
            __folded = __reduce(__folded, __shuffled4);

            return _mm_cvtsi128_si32(__folded);
        }
    }
};

template <class _DesiredType_>
struct _Simd_fold<arch::ISA::AVX2, 256, _DesiredType_> {
    template <
        class _IntrinType_,
        class _ReduceBinaryFunction_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_            __vector,
        _ReduceBinaryFunction_  __reduce) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 8) {
            auto __folded = __intrin_bitcast<__m256d>(__vector);
        
            const auto __shuffle1 = _mm256_shuffle_pd(__folded, __folded, 5);
            __folded = __reduce(__folded, __shuffle1);

            const auto __shuffle2 = _mm256_permute4x64_epi64(__intrin_bitcast<__m256i>(__folded), 0x1B);
            __folded = __intrin_bitcast<__m256d>(__reduce(
                __intrin_bitcast<__m256i>(__folded), __intrin_bitcast<__m256i>(__shuffle2)));

            if constexpr (__is_pd_v<_DesiredType_>)
                return _mm256_cvtsd_f64(__folded);
            else 
                return _mm256_cvtsi256_si64(__intrin_bitcast<__m256i>(__folded));
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            auto __folded = __intrin_bitcast<__m256i>(__vector);

            const auto __shuffle1 = _mm256_permute4x64_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffle1);

            const auto __shuffle2 = _mm256_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffle2);

            const auto __shuffle3 = _mm256_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffle3);

            if constexpr (__is_ps_v<_DesiredType_>)
                return _mm256_cvtss_f32(__intrin_bitcast<__m256>(__folded));
            else 
                return _mm256_cvtsi256_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            const auto __shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));

            auto __folded = __intrin_bitcast<__m256i>(__vector);

            const auto __shuffle1 = _mm256_permute4x64_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffle1);

            const auto __shuffle2 = _mm256_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffle2);

            const auto __shuffle3 = _mm256_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffle3);

            const auto __shuffle4 = _mm256_shuffle_epi8(__folded, __shuffle_words);
            __folded = __reduce(__folded, __shuffle4);

            return _mm256_cvtsi256_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
            const auto __shuffle_bytes = _mm256_broadcastsi128_si256(_mm_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1));

            auto __folded = __intrin_bitcast<__m256i>(__vector);
      
            const auto __shuffle1 = _mm256_permute4x64_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffle1);

            const auto __shuffle2 = _mm256_shuffle_epi32(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffle2);

            const auto __shuffle3 = _mm256_shuffle_epi32(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffle3);

            const auto __shuffle4 = _mm256_shuffle_epi8(__folded, __shuffle_words);
            __folded = __reduce(__folded, __shuffle4);

            const auto __shuffle5 = _mm256_shuffle_epi8(__folded, __shuffle_bytes);
            __folded = __reduce(__folded, __shuffle5);

            return _mm256_cvtsi256_si32(__folded);
        }
    }
};

template <class _DesiredType_>
struct _Simd_fold<arch::ISA::AVX512F, 512, _DesiredType_> {
    template <
        class _IntrinType_,
        class _ReduceBinaryFunction_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_            __vector,
        _ReduceBinaryFunction_  __reduce) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 8) {
            auto __folded = __intrin_bitcast<__m512i>(__vector);

            const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled3);

            if constexpr (__is_pd_v<_DesiredType_>)
                return _mm512_cvtsd_f64(__intrin_bitcast<__m512d>(__folded));
            else
                return _mm512_cvtsi512_si64(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            auto __folded = __intrin_bitcast<__m512i>(__vector);

            const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled3);

            const auto __shuffled4 = __intrin_bitcast<__m512i>(_mm512_permute_ps(__intrin_bitcast<__m512>(__folded), 0xB1));
            __folded = __reduce(__folded, __shuffled4);

            if constexpr (__is_ps_v<_DesiredType_>)
                return _mm512_cvtss_f32(__intrin_bitcast<__m512>(__folded));
            else
                return _mm512_cvtsi512_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            const auto __shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
            auto __folded = __intrin_bitcast<__m512i>(__vector);

            const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled3);

            const auto __shuffled4 = __intrin_bitcast<__m512i>(_mm512_permute_ps(__intrin_bitcast<__m512>(__folded), 0xB1));
            __folded = __reduce(__folded, __shuffled4);

            const auto __shuffled5_low  = _mm256_shuffle_epi8(__intrin_bitcast<__m256i>(__folded), __shuffle_words);
            const auto __shuffled5_high = _mm256_shuffle_epi8(__intrin_bitcast<__m256i>(_mm512_extractf64x4_pd(
                __intrin_bitcast<__m512d>(__folded), 1)), __shuffle_words);
        
            auto __shuffled5 = __intrin_bitcast<__m512d>(__shuffled5_low);
            __shuffled5 = _mm512_insertf64x4(__shuffled5, __intrin_bitcast<__m256d>(__shuffled5_high), 1);

            __folded = __reduce(__folded, __intrin_bitcast<__m512i>(__shuffled5));

            return _mm512_cvtsi512_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
            const auto __shuffle_bytes = _mm256_broadcastsi128_si256(_mm_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1));

            auto __folded = __intrin_bitcast<__m512i>(__vector);

            const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled3);

            const auto __shuffled4 = __intrin_bitcast<__m512i>(_mm512_permute_ps(__intrin_bitcast<__m512>(__folded), 0xB1));
            __folded = __reduce(__folded, __shuffled4);

            const auto __shuffled5_low  = _mm256_shuffle_epi8(__intrin_bitcast<__m256i>(__folded), __shuffle_words);
            const auto __shuffled5_high = _mm256_shuffle_epi8(__intrin_bitcast<__m256i>(_mm512_extractf64x4_pd(
                __intrin_bitcast<__m512d>(__folded), 1)), __shuffle_words);
        
            auto __shuffled5 = __intrin_bitcast<__m512d>(__shuffled5_low);
            __shuffled5 = _mm512_insertf64x4(__shuffled5, __intrin_bitcast<__m256d>(__shuffled5_high), 1);

            __folded = __reduce(__folded, __intrin_bitcast<__m512i>(__shuffled5));

            const auto __shuffled6_low  = _mm256_shuffle_epi8(__intrin_bitcast<__m256i>(__folded), __shuffle_bytes);
            const auto __shuffled6_high = _mm256_shuffle_epi8(__intrin_bitcast<__m256i>(_mm512_extractf64x4_pd(
                __intrin_bitcast<__m512d>(__folded), 1)), __shuffle_bytes);

            auto __shuffled6 = __intrin_bitcast<__m512d>(__shuffled6_low);
            __shuffled6 = _mm512_insertf64x4(__shuffled6, __intrin_bitcast<__m256d>(__shuffled6_high), 1);

            __folded = __reduce(__folded, __intrin_bitcast<__m512i>(__shuffled6));

            return _mm512_cvtsi512_si32(__folded);
        }
    }
};

template <class _DesiredType_>
struct _Simd_fold<arch::ISA::AVX512BW, 512, _DesiredType_> :
    _Simd_fold<arch::ISA::AVX512F, 512, _DesiredType_>
{
    template <
        class _IntrinType_,
        class _ReduceBinaryFunction_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_            __vector,
        _ReduceBinaryFunction_  __reduce) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) >= 4) {
            return _Simd_fold<arch::ISA::AVX512F, 512, _DesiredType_>()(__vector, __reduce);
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            const auto __shuffle_words = _mm512_set_epi8(
                61, 60, 63, 62, 57, 56, 59, 58, 53, 52, 55, 54, 49, 48, 51, 50,
                45, 44, 47, 46, 41, 40, 43, 42, 37, 36, 39, 38, 33, 32, 35, 34,
                29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18,
                13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

            auto __folded = __intrin_bitcast<__m512i>(__vector);

            const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled3);

            const auto __shuffled4 = __intrin_bitcast<__m512i>(_mm512_permute_ps(__intrin_bitcast<__m512>(__folded), 0xB1));
            __folded = __reduce(__folded, __shuffled4);

            const auto __shuffled5 = _mm512_shuffle_epi8(__folded, __shuffle_words);
            __folded = __reduce(__folded, __intrin_bitcast<__m512i>(__shuffled5));

            return _mm512_cvtsi512_si32(__folded);
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __shuffle_words = _mm512_set_epi8(
                61, 60, 63, 62, 57, 56, 59, 58, 53, 52, 55, 54, 49, 48, 51, 50,
                45, 44, 47, 46, 41, 40, 43, 42, 37, 36, 39, 38, 33, 32, 35, 34,
                29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18,
                13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

            const auto __shuffle_bytes = _mm512_set_epi8(
                62, 63, 60, 61, 58, 59, 56, 57, 54, 55, 52, 53, 50, 51, 48, 49,
                46, 47, 44, 45, 42, 43, 40, 41, 38, 39, 36, 37, 34, 35, 32, 33,
                30, 31, 28, 29, 26, 27, 24, 25, 22, 23, 20, 21, 18, 19, 16, 17,
                14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1);

            auto __folded = __intrin_bitcast<__m512i>(__vector);

            const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
            __folded = __reduce(__folded, __shuffled1);

            const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
            __folded = __reduce(__folded, __shuffled2);

            const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
            __folded = __reduce(__folded, __shuffled3);

            const auto __shuffled4 = __intrin_bitcast<__m512i>(_mm512_permute_ps(__intrin_bitcast<__m512>(__folded), 0xB1));
            __folded = __reduce(__folded, __shuffled4);

            const auto __shuffled5 = _mm512_shuffle_epi8(__folded, __shuffle_words);
            __folded = __reduce(__folded, __intrin_bitcast<__m512i>(__shuffled5));

            const auto __shuffled6 = _mm512_shuffle_epi8(__folded, __shuffle_bytes);
            __folded = __reduce(__folded, __intrin_bitcast<__m512i>(__shuffled6));

            return _mm512_cvtsi512_si32(__folded);
        }
    }
};

template <class _DesiredType_> struct _Simd_fold<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_fold<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_fold<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_fold<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_fold<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_fold<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_fold<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_fold<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_fold<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_fold<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_fold<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
