#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, class Reduce>
raze_always_inline T mirror_fold_(V x, Reduce reduce) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (sizeof(T) == 8) {
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 8)));
            if constexpr (pd<T>) return _mm_cvtsd_f64(as<__m128d>(x));
            else return _mm_cvtsi128_si64(as<__m128i>(x));
        }
        else if constexpr (sizeof(T) == 4) {
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 8)));
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 4)));
            if constexpr (ps<T>) return _mm_cvtss_f32(as<__m128>(x));
            else return _mm_cvtsi128_si32(as<__m128i>(x));
        }
        else if constexpr (sizeof(T) == 2) {
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 8)));
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 4)));
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 2)));
            return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(x)));
        }
        else if constexpr (sizeof(T) == 1) {
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 8)));
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 4)));
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 2)));
            x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 1)));
            return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(x)));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (sizeof(T) == 8) {
            auto folded = as<__m256d>(x);

            const auto shuffle1 = _mm256_shuffle_pd(folded, folded, 5);
            folded = reduce(folded, shuffle1);

            const auto swapped_halfs = as<__m256>(_mm256_permute2f128_pd(folded, folded, 1));
            const auto shuffle2 = _mm256_shuffle_ps(swapped_halfs, swapped_halfs, 0x4E);
            folded = reduce(folded, as<__m256d>(shuffle2));

            if constexpr (pd<T>) return _mm256_cvtsd_f64(folded);
            else return _mm256_cvtsi256_si64(as<__m256i>(folded));
        }
        else if constexpr (sizeof(T) == 4) {
            auto folded = as<__m256>(x);

            const auto swapped_halfs = as<__m256>(_mm256_permute2f128_ps(folded, folded, 1));
            const auto shuffle1 = _mm256_shuffle_ps(swapped_halfs, swapped_halfs, 0x4E);
            folded = reduce(folded, shuffle1);

            const auto shuffle2 = _mm256_shuffle_ps(folded, folded, 0x4E);
            folded = reduce(folded, shuffle2);

            const auto shuffle3 = _mm256_shuffle_ps(folded, folded, 0xB1);
            folded = reduce(folded, shuffle3);

            if constexpr (ps<T>) return _mm256_cvtss_f32(as<__m256>(folded));
            else return _mm256_cvtsi256_si32(as<__m256i>(folded));
        }
        else if constexpr (sizeof(T) == 2) {
            const auto high = _mm256_permute2f128_pd(as<__m256d>(x), as<__m256d>(x), 1);
            auto folded = as<__m256d>(x);
            folded = reduce(folded, high);

            auto low = as<__m128d>(folded);

            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 8)));
            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 4)));
            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 2)));

            return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(low)));
        }
        else if constexpr (sizeof(T) == 1) {
            const auto high = _mm256_permute2f128_pd(as<__m256d>(x), as<__m256d>(x), 1);

            auto folded = as<__m256d>(x);
            folded = reduce(folded, high);

            auto low = as<__m128d>(folded);

            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 8)));
            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 4)));
            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 2)));
            low = reduce(low, as<__m128d>(_mm_srli_si128(as<__m128i>(low), 1)));

            return static_cast<T>(_mm_cvtsi128_si32(as<__m128i>(low)));
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (sizeof(T) == 8) {
            auto folded = as<__m512i>(x);

            const auto shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), folded);
            folded = reduce(folded, shuffled1);

            const auto shuffled2 = _mm512_permutex_epi64(folded, 0x4E);
            folded = reduce(folded, shuffled2);

            const auto shuffled3 = _mm512_permutex_epi64(folded, 0xB1);
            folded = reduce(folded, shuffled3);

            if constexpr (pd<T>)
                return _mm512_cvtsd_f64(as<__m512d>(folded));
            else
                return _mm512_cvtsi512_si64(folded);
        }
        else if constexpr (sizeof(T) == 4) {
            auto folded = as<__m512i>(x);

            const auto shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), folded);
            folded = reduce(folded, shuffled1);

            const auto shuffled2 = _mm512_permutex_epi64(folded, 0x4E);
            folded = reduce(folded, shuffled2);

            const auto shuffled3 = _mm512_permutex_epi64(folded, 0xB1);
            folded = reduce(folded, shuffled3);

            const auto shuffled4 = as<__m512i>(_mm512_permute_ps(as<__m512>(folded), 0xB1));
            folded = reduce(folded, shuffled4);

            if constexpr (ps<T>)
                return _mm512_cvtss_f32(as<__m512>(folded));
            else
                return _mm512_cvtsi512_si32(folded);
        }
        else if constexpr (sizeof(T) == 2) {
            if constexpr (has_avx512bw<ISA>) {
                auto shuffled = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), as<__m512i>(x));
                x = reduce(x, as<V>(shuffled));

                shuffled = _mm512_permutex_epi64(as<__m512i>(x), 0x4E);
                x = reduce(x, as<V>(shuffled));

                shuffled = _mm512_permutex_epi64(as<__m512i>(x), 0xB1);
                x = reduce(x, as<V>(shuffled));

                shuffled = as<__m512i>(_mm512_permute_ps(as<__m512>(x), 0xB1));
                x = reduce(x, as<V>(shuffled));

                shuffled = _mm512_shuffle_epi8(as<__m512i>(x), _mm512_set_epi8(
                    61, 60, 63, 62, 57, 56, 59, 58, 53, 52, 55, 54, 49, 48, 51, 50,
                    45, 44, 47, 46, 41, 40, 43, 42, 37, 36, 39, 38, 33, 32, 35, 34,
                    29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18,
                    13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
                x = reduce(x, as<V>(shuffled));

                return _mm512_cvtsi512_si32(as<__m512i>(x));
            }
            else {
                const auto shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
                auto folded = as<__m512i>(x);

                const auto shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), folded);
                folded = reduce(folded, shuffled1);

                const auto shuffled2 = _mm512_permutex_epi64(folded, 0x4E);
                folded = reduce(folded, shuffled2);

                const auto shuffled3 = _mm512_permutex_epi64(folded, 0xB1);
                folded = reduce(folded, shuffled3);

                const auto shuffled4 = as<__m512i>(_mm512_permute_ps(as<__m512>(folded), 0xB1));
                folded = reduce(folded, shuffled4);

                const auto shuffled5_low = _mm256_shuffle_epi8(as<__m256i>(folded), shuffle_words);
                const auto shuffled5_high = _mm256_shuffle_epi8(as<__m256i>(_mm512_extractf64x4_pd(
                    as<__m512d>(folded), 1)), shuffle_words);

                auto shuffled5 = as<__m512d>(shuffled5_low);
                shuffled5 = _mm512_insertf64x4(shuffled5, as<__m256d>(shuffled5_high), 1);

                folded = reduce(folded, as<__m512i>(shuffled5));

                return _mm512_cvtsi512_si32(folded);
            }
        }
        else if constexpr (sizeof(T) == 1) {
            if constexpr (has_avx512bw<ISA>) {
                const auto shuffle_words = _mm512_set_epi8(
                    61, 60, 63, 62, 57, 56, 59, 58, 53, 52, 55, 54, 49, 48, 51, 50,
                    45, 44, 47, 46, 41, 40, 43, 42, 37, 36, 39, 38, 33, 32, 35, 34,
                    29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18,
                    13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

                const auto shuffle_bytes = _mm512_set_epi8(
                    62, 63, 60, 61, 58, 59, 56, 57, 54, 55, 52, 53, 50, 51, 48, 49,
                    46, 47, 44, 45, 42, 43, 40, 41, 38, 39, 36, 37, 34, 35, 32, 33,
                    30, 31, 28, 29, 26, 27, 24, 25, 22, 23, 20, 21, 18, 19, 16, 17,
                    14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1);

                auto folded = as<__m512i>(x);

                const auto shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), folded);
                folded = reduce(folded, shuffled1);

                const auto shuffled2 = _mm512_permutex_epi64(folded, 0x4E);
                folded = reduce(folded, shuffled2);

                const auto shuffled3 = _mm512_permutex_epi64(folded, 0xB1);
                folded = reduce(folded, shuffled3);

                const auto shuffled4 = as<__m512i>(_mm512_permute_ps(as<__m512>(folded), 0xB1));
                folded = reduce(folded, shuffled4);

                const auto shuffled5 = _mm512_shuffle_epi8(folded, shuffle_words);
                folded = reduce(folded, as<__m512i>(shuffled5));

                const auto shuffled6 = _mm512_shuffle_epi8(folded, shuffle_bytes);
                folded = reduce(folded, as<__m512i>(shuffled6));

                return _mm512_cvtsi512_si32(folded);
            }
            else {
                const auto shuffle_words = _mm256_broadcastsi128_si256(_mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2));
                const auto shuffle_bytes = _mm256_broadcastsi128_si256(_mm_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1));

                auto folded = as<__m512i>(x);

                const auto shuffled1 = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), folded);
                folded = reduce(folded, shuffled1);

                const auto shuffled2 = _mm512_permutex_epi64(folded, 0x4E);
                folded = reduce(folded, shuffled2);

                const auto shuffled3 = _mm512_permutex_epi64(folded, 0xB1);
                folded = reduce(folded, shuffled3);

                const auto shuffled4 = as<__m512i>(_mm512_permute_ps(as<__m512>(folded), 0xB1));
                folded = reduce(folded, shuffled4);

                const auto shuffled5_low = _mm256_shuffle_epi8(as<__m256i>(folded), shuffle_words);
                const auto shuffled5_high = _mm256_shuffle_epi8(as<__m256i>(_mm512_extractf64x4_pd(
                    as<__m512d>(folded), 1)), shuffle_words);

                auto shuffled5 = as<__m512d>(shuffled5_low);
                shuffled5 = _mm512_insertf64x4(shuffled5, as<__m256d>(shuffled5_high), 1);

                folded = reduce(folded, as<__m512i>(shuffled5));

                const auto shuffled6_low = _mm256_shuffle_epi8(as<__m256i>(folded), shuffle_bytes);
                const auto shuffled6_high = _mm256_shuffle_epi8(as<__m256i>(_mm512_extractf64x4_pd(
                    as<__m512d>(folded), 1)), shuffle_bytes);

                auto shuffled6 = as<__m512d>(shuffled6_low);
                shuffled6 = _mm512_insertf64x4(shuffled6, as<__m256d>(shuffled6_high), 1);

                folded = reduce(folded, as<__m512i>(shuffled6));

                return _mm512_cvtsi512_si32(folded);
            }
        }
    }
    else {
        return x;
    }
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M, class Reduce>
raze_always_inline T mirror_fold_(V x, Reduce reduce, M mask) noexcept {
    return mirror_fold_<ISA, T>(select_<ISA, T>(x, mask), reduce);
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M, class Reduce>
raze_always_inline T mirror_fold_(V x, Reduce reduce, M mask, V src) noexcept {
    return mirror_fold_<ISA, T>(select_<ISA, T>(x, src, mask), reduce);
}

__RAZE_VX_NAMESPACE_END
