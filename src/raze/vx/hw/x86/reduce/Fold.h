#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Fold {
    template <intrin_or_arithmetic_type _Tp_, class _ReduceBinaryFunction_>
    raze_nodiscard raze_static_operator raze_always_inline _Type_ operator()(
        _Tp_ __x, _ReduceBinaryFunction_  __reduce) raze_const_operator noexcept
    {
        if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (sizeof(_Type_) == 8) {
                auto __folded = __as<__m128i>(__x);
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 8));

                if constexpr (__is_pd_v<_Type_>) return _mm_cvtsd_f64(__as<__m128d>(__folded));
                else return _mm_cvtsi128_si64(__folded);
            }
            else if constexpr (sizeof(_Type_) == 4) {
                auto __folded = __as<__m128i>(__x);

                __folded = __reduce(__folded, _mm_srli_si128(__folded, 8));
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 4));

                if constexpr (__is_ps_v<_Type_>) return _mm_cvtss_f32(__as<__m128>(__folded));
                else return _mm_cvtsi128_si32(__folded);
            }
            else if constexpr (sizeof(_Type_) == 2) {
                auto __folded = __as<__m128i>(__x);

                __folded = __reduce(__folded, _mm_srli_si128(__folded, 8));
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 4));
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 2));

                return static_cast<_Type_>(_mm_cvtsi128_si32(__as<__m128i>(__folded)));
            }
            else if constexpr (sizeof(_Type_) == 1) {
                auto __folded = __as<__m128i>(__x);

                __folded = __reduce(__folded, _mm_srli_si128(__folded, 8));
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 4));
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 2));
                __folded = __reduce(__folded, _mm_srli_si128(__folded, 1));

                return static_cast<_Type_>(_mm_cvtsi128_si32(__as<__m128i>(__folded)));
            }
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            if constexpr (sizeof(_Type_) == 8) {
                auto __folded = __as<__m256d>(__x);

                const auto __shuffle1 = _mm256_shuffle_pd(__folded, __folded, 5);
                __folded = __reduce(__folded, __shuffle1);

                const auto __swapped_halfs = __as<__m256>(_mm256_permute2f128_pd(__folded, __folded, 1));
                const auto __shuffle2 = _mm256_shuffle_ps(__swapped_halfs, __swapped_halfs, 0x4E);
                __folded = __reduce(__folded, __as<__m256d>(__shuffle2));

                if constexpr (__is_pd_v<_Type_>)
                    return _mm256_cvtsd_f64(__folded);
                else
                    return _mm256_cvtsi256_si64(__as<__m256i>(__folded));
            }
            else if constexpr (sizeof(_Type_) == 4) {
                auto __folded = __as<__m256>(__x);

                const auto __swapped_halfs = __as<__m256>(_mm256_permute2f128_ps(__folded, __folded, 1));
                const auto __shuffle1 = _mm256_shuffle_ps(__swapped_halfs, __swapped_halfs, 0x4E);
                __folded = __reduce(__folded, __shuffle1);

                const auto __shuffle2 = _mm256_shuffle_ps(__folded, __folded, 0x4E);
                __folded = __reduce(__folded, __shuffle2);

                const auto __shuffle3 = _mm256_shuffle_ps(__folded, __folded, 0xB1);
                __folded = __reduce(__folded, __shuffle3);

                if constexpr (__is_ps_v<_Type_>)
                    return _mm256_cvtss_f32(__as<__m256>(__folded));
                else
                    return _mm256_cvtsi256_si32(__as<__m256i>(__folded));
            }
            else if constexpr (sizeof(_Type_) == 2) {
                const auto __high = _mm256_permute2f128_pd(__as<__m256d>(__x),
                    __as<__m256d>(__x), 1);

                auto __folded = __as<__m256d>(__x);
                __folded = __reduce(__folded, __high);

                auto __low = __as<__m128d>(__folded);

                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 8)));
                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 4)));
                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 2)));

                return static_cast<_Type_>(_mm_cvtsi128_si32(__as<__m128i>(__low)));
            }
            else if constexpr (sizeof(_Type_) == 1) {
                const auto __high = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 1);

                auto __folded = __as<__m256d>(__x);
                __folded = __reduce(__folded, __high);

                auto __low = __as<__m128d>(__folded);

                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 8)));
                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 4)));
                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 2)));
                __low = __reduce(__low, __as<__m128d>(_mm_srli_si128(__as<__m128i>(__low), 1)));

                return static_cast<_Type_>(_mm_cvtsi128_si32(__as<__m128i>(__low)));
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (sizeof(_Type_) == 8) {
                auto __folded = __as<__m512i>(__x);

                const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
                __folded = __reduce(__folded, __shuffled1);

                const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
                __folded = __reduce(__folded, __shuffled2);

                const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
                __folded = __reduce(__folded, __shuffled3);

                if constexpr (__is_pd_v<_Type_>)
                    return _mm512_cvtsd_f64(__as<__m512d>(__folded));
                else
                    return _mm512_cvtsi512_si64(__folded);
            }
            else if constexpr (sizeof(_Type_) == 4) {
                auto __folded = __as<__m512i>(__x);

                const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
                __folded = __reduce(__folded, __shuffled1);

                const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
                __folded = __reduce(__folded, __shuffled2);

                const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
                __folded = __reduce(__folded, __shuffled3);

                const auto __shuffled4 = __as<__m512i>(_mm512_permute_ps(__as<__m512>(__folded), 0xB1));
                __folded = __reduce(__folded, __shuffled4);

                if constexpr (__is_ps_v<_Type_>)
                    return _mm512_cvtss_f32(__as<__m512>(__folded));
                else
                    return _mm512_cvtsi512_si32(__folded);
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__has_avx512bw_support_v<_ISA_>) {
                    const auto __shuffle_words = _mm512_set_epi8(
                        61, 60, 63, 62, 57, 56, 59, 58, 53, 52, 55, 54, 49, 48, 51, 50,
                        45, 44, 47, 46, 41, 40, 43, 42, 37, 36, 39, 38, 33, 32, 35, 34,
                        29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18,
                        13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

                    auto __folded = __as<__m512i>(__x);

                    const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
                    __folded = __reduce(__folded, __shuffled1);

                    const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
                    __folded = __reduce(__folded, __shuffled2);

                    const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
                    __folded = __reduce(__folded, __shuffled3);

                    const auto __shuffled4 = __as<__m512i>(_mm512_permute_ps(__as<__m512>(__folded), 0xB1));
                    __folded = __reduce(__folded, __shuffled4);

                    const auto __shuffled5 = _mm512_shuffle_epi8(__folded, __shuffle_words);
                    __folded = __reduce(__folded, __as<__m512i>(__shuffled5));

                    return _mm512_cvtsi512_si32(__folded);
                }
                else {
                    const auto __shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
                    auto __folded = __as<__m512i>(__x);

                    const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
                    __folded = __reduce(__folded, __shuffled1);

                    const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
                    __folded = __reduce(__folded, __shuffled2);

                    const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
                    __folded = __reduce(__folded, __shuffled3);

                    const auto __shuffled4 = __as<__m512i>(_mm512_permute_ps(__as<__m512>(__folded), 0xB1));
                    __folded = __reduce(__folded, __shuffled4);

                    const auto __shuffled5_low = _mm256_shuffle_epi8(__as<__m256i>(__folded), __shuffle_words);
                    const auto __shuffled5_high = _mm256_shuffle_epi8(__as<__m256i>(_mm512_extractf64x4_pd(
                        __as<__m512d>(__folded), 1)), __shuffle_words);

                    auto __shuffled5 = __as<__m512d>(__shuffled5_low);
                    __shuffled5 = _mm512_insertf64x4(__shuffled5, __as<__m256d>(__shuffled5_high), 1);

                    __folded = __reduce(__folded, __as<__m512i>(__shuffled5));

                    return _mm512_cvtsi512_si32(__folded);
                }
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__has_avx512bw_support_v<_ISA_>) {
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

                    auto __folded = __as<__m512i>(__x);

                    const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
                    __folded = __reduce(__folded, __shuffled1);

                    const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
                    __folded = __reduce(__folded, __shuffled2);

                    const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
                    __folded = __reduce(__folded, __shuffled3);

                    const auto __shuffled4 = __as<__m512i>(_mm512_permute_ps(__as<__m512>(__folded), 0xB1));
                    __folded = __reduce(__folded, __shuffled4);

                    const auto __shuffled5 = _mm512_shuffle_epi8(__folded, __shuffle_words);
                    __folded = __reduce(__folded, __as<__m512i>(__shuffled5));

                    const auto __shuffled6 = _mm512_shuffle_epi8(__folded, __shuffle_bytes);
                    __folded = __reduce(__folded, __as<__m512i>(__shuffled6));

                    return _mm512_cvtsi512_si32(__folded);
                }
                else {
                    const auto __shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
                    const auto __shuffle_bytes = _mm256_broadcastsi128_si256(_mm_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1));

                    auto __folded = __as<__m512i>(__x);

                    const auto __shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), __folded);
                    __folded = __reduce(__folded, __shuffled1);

                    const auto __shuffled2 = _mm512_permutex_epi64(__folded, 0x4E);
                    __folded = __reduce(__folded, __shuffled2);

                    const auto __shuffled3 = _mm512_permutex_epi64(__folded, 0xB1);
                    __folded = __reduce(__folded, __shuffled3);

                    const auto __shuffled4 = __as<__m512i>(_mm512_permute_ps(__as<__m512>(__folded), 0xB1));
                    __folded = __reduce(__folded, __shuffled4);

                    const auto __shuffled5_low = _mm256_shuffle_epi8(__as<__m256i>(__folded), __shuffle_words);
                    const auto __shuffled5_high = _mm256_shuffle_epi8(__as<__m256i>(_mm512_extractf64x4_pd(
                        __as<__m512d>(__folded), 1)), __shuffle_words);

                    auto __shuffled5 = __as<__m512d>(__shuffled5_low);
                    __shuffled5 = _mm512_insertf64x4(__shuffled5, __as<__m256d>(__shuffled5_high), 1);

                    __folded = __reduce(__folded, __as<__m512i>(__shuffled5));

                    const auto __shuffled6_low = _mm256_shuffle_epi8(__as<__m256i>(__folded), __shuffle_bytes);
                    const auto __shuffled6_high = _mm256_shuffle_epi8(__as<__m256i>(_mm512_extractf64x4_pd(
                        __as<__m512d>(__folded), 1)), __shuffle_bytes);

                    auto __shuffled6 = __as<__m512d>(__shuffled6_low);
                    __shuffled6 = _mm512_insertf64x4(__shuffled6, __as<__m256d>(__shuffled6_high), 1);

                    __folded = __reduce(__folded, __as<__m512i>(__shuffled6));

                    return _mm512_cvtsi512_si32(__folded);
                }
            }
        }
        else {
            return __x;
        }
    }
};


__RAZE_VX_NAMESPACE_END
