 #pragma once 

#include <src/raze/vx/hw/x86/reduce/ReduceAddType.h>
#include <src/raze/vx/hw/x86/arithmetic/Add.h>
#include <src/raze/vx/hw/x86/access/Extract.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline reduce_type<T> reduce_add_(V x) noexcept {
    using ReduceType = reduce_type<T>;

    if constexpr (sizeof(V) == 16) {
        if constexpr (epi64<T> || epu64<T>) {
#if defined(raze_processor_x86_32)
            return static_cast<ReduceType>(_mm_cvtsi128_si32(as<__m128i>(x)) +
                extract_<arch::ISA::SSE2, i32>(x, 2));
#else 
            return static_cast<ReduceType>(_mm_cvtsi128_si64(as<__m128i>(x)) +
                extract_<arch::ISA::SSE2, i64>(x, 1));
#endif // defined(raze_processor_x86_32)
        }
        else if constexpr (epi32<T> || epu32<T>) {
            if constexpr (has_ssse3<ISA>) {
                const auto reduce4 = _mm_hadd_epi32(as<__m128i>(x), as<__m128i>(x));
                return static_cast<ReduceType>(_mm_cvtsi128_si32(_mm_hadd_epi32(reduce4, reduce4)));
            }
            else {
                const auto shuffled1 = _mm_shuffle_epi32(as<__m128i>(x), _MM_SHUFFLE(2, 3, 0, 1));
                const auto reduce1 = _mm_add_epi32(as<__m128i>(x), shuffled1);

                const auto shuffled2 = _mm_shuffle_epi32(reduce1, _MM_SHUFFLE(1, 0, 3, 2));
                const auto reduce2 = _mm_add_epi32(reduce1, shuffled2);

                return _mm_cvtsi128_si32(as<__m128i>(reduce2));
            }
        }
        else if constexpr (epi16<T> || epu16<T>) {
            if constexpr (has_ssse3<ISA>) {
                const auto zeros = _mm_setzero_si128();

                const auto reduce2 = _mm_hadd_epi16(as<__m128i>(x), zeros);
                const auto reduce3 = _mm_unpacklo_epi16(reduce2, zeros);

                const auto reduce4 = _mm_hadd_epi32(reduce3, zeros);
                const auto reduce5 = _mm_hadd_epi32(reduce4, zeros);

                return static_cast<ReduceType>(_mm_cvtsi128_si32(reduce5));
            }
            else {
                const auto zeros = _mm_setzero_si128();

                const auto low = _mm_unpacklo_epi16(as<__m128i>(x), zeros);
                const auto high = _mm_unpackhi_epi16(as<__m128i>(x), zeros);

                const auto reduce1 = _mm_add_epi32(low, high);

                const auto shuffle1 = _mm_shuffle_epi32(reduce1, _MM_SHUFFLE(2, 3, 0, 1));
                const auto reduce2 = _mm_add_epi32(reduce1, shuffle1);

                const auto shuffled2 = _mm_shuffle_epi32(reduce2, _MM_SHUFFLE(1, 0, 3, 2));
                return _mm_cvtsi128_si32(_mm_add_epi32(reduce2, shuffled2));
            }
        }
        else if constexpr (epi8<T> || epu8<T>) {
            const auto first_reduce = _mm_sad_epu8(as<__m128i>(x), _mm_setzero_si128());
#if defined(raze_processor_x86_32)
            return static_cast<ReduceType>(_mm_cvtsi128_si32(first_reduce)
                + extract_<arch::ISA::SSE2, i32>(first_reduce, 2));
#else
            return static_cast<_ReduceType>(_mm_cvtsi128_si64(as<__m128i>(first_reduce))
                + extract_<arch::ISA::SSE2, i64>(first_reduce, 1));
#endif // defined(raze_processor_x86_32)
        }
        else if constexpr (ps<T>) {
            if constexpr (has_sse3<ISA>) {
                const auto reduce4 = _mm_hadd_ps(as<__m128>(x), as<__m128>(x));
                return static_cast<ReduceType>(_mm_cvtss_f32(_mm_hadd_ps(reduce4, reduce4)));
            }
            else {
                const auto shuffled1 = _mm_movehl_ps(as<__m128>(x), as<__m128>(x));
                const auto reduce1 = _mm_add_ps(as<__m128>(x), shuffled1);

                const auto shuffled2 = _mm_shuffle_ps(reduce1, reduce1, 0x1);
                return _mm_cvtss_f32(_mm_add_ss(reduce1, shuffled2));
            }
        }
        else if constexpr (pd<T>) {
            if constexpr (has_sse3<ISA>) return _mm_cvtsd_f64(_mm_hadd_pd(as<__m128d>(x), as<__m128d>(x)));
            else return _mm_cvtsd_f64(_mm_add_sd(as<__m128d>(x), _mm_unpackhi_pd(as<__m128d>(x), as<__m128d>(x))));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (has_avx2<ISA>) {
            if constexpr (epi64<T> || epu64<T>) {
                const auto low64 = as<__m128i>(x);
                const auto high64 = _mm256_extracti128_si256(as<__m256i>(x), 1);

                const auto reduce = _mm_add_epi64(low64, high64);
                return reduce_add_<arch::ISA::SSSE3, T>(reduce);
            }
            else if constexpr (epi32<T> || epu32<T>) {
                const auto zeros = _mm256_setzero_si256();

                const auto reduce4 = _mm256_hadd_epi32(as<__m256i>(x), zeros);
                const auto reduce5 = _mm256_permute4x64_epi64(reduce4, 0xD8);

                const auto reduce6 = _mm256_hadd_epi32(reduce5, zeros);
                const auto reduce7 = _mm256_hadd_epi32(reduce6, zeros);

                return static_cast<ReduceType>(_mm_cvtsi128_si32(as<__m128i>(__reduce7)));
            }
            else if constexpr (epi16<T> || epu16<T>) {
                const auto zeros = _mm256_setzero_si256();

                const auto reduce2 = _mm256_hadd_epi16(as<__m256i>(x), zeros);
                const auto reduce3 = _mm256_unpacklo_epi16(reduce2, zeros);

                const auto reduce4 = _mm256_hadd_epi32(reduce3, zeros);
                const auto reduce5 = _mm256_permute4x64_epi64(reduce4, 0xD8);

                const auto reduce6 = _mm256_hadd_epi32(reduce5, zeros);
                const auto reduce7 = _mm256_hadd_epi32(reduce6, zeros);

                return static_cast<ReduceType>(_mm_cvtsi128_si32(as<__m128i>(reduce7)));
            }
            else if constexpr (epi8<T> || epu8<T>) {
                const auto reduce1 = _mm256_sad_epu8(as<__m256i>(x), _mm256_setzero_si256());

                const auto low64 = _mm256_castsi256_si128(reduce1);
                const auto high64 = _mm256_extracti128_si256(reduce1, 1);

                const auto reduce8 = _mm_add_epi64(low64, high64);
                return reduce_add_<arch::ISA::SSSE3, i64>(reduce8);
            }
            else if constexpr (pd<T>) {
                const auto low64 = as<__m128d>(x);
                const auto high64 = _mm256_extractf128_pd(as<__m256d>(x), 1);

                const auto reduce = _mm_add_pd(low64, high64);
                return reduce_add_<arch::ISA::SSSE3, T>(reduce);
            }
            else if constexpr (ps<T>) {
                const auto zeros = _mm256_setzero_ps();

                const auto reduce4 = _mm256_hadd_ps(as<__m256>(x), zeros);
                const auto reduce5 = as<__m256>(_mm256_permute4x64_epi64(as<__m256i>(reduce4), 0xD8));

                const auto reduce6 = _mm256_hadd_ps(reduce5, zeros);
                const auto reduce7 = _mm256_hadd_ps(reduce6, zeros);

                return static_cast<ReduceType>(_mm_cvtss_f32(as<__m128>(reduce7)));
            }
        }
        else {
            const auto low = as<__m128>(x);
            const auto high = _mm256_extractf128_ps(as<__m256>(x), 1);

            const auto vertical_sum = add_<arch::ISA::AVX, T>(low, high);
            return _Reduce_add<arch::ISA::AVX, T>()(vertical_sum);
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (epi64<T> || epu64<T>) return _mm512_reduce_add_epi64(as<__m512i>(x));
        else if constexpr (epi32<T> || epu32<T>) return _mm512_reduce_add_epi32(as<__m512i>(x));
        else if constexpr (ps<T>) return _mm512_reduce_add_ps(as<__m512>(x));
        else if constexpr (pd<T>)  return _mm512_reduce_add_pd(as<__m512d>(x));
        else if constexpr (epi8<T> || epu8<T>)
            return reduce_add_<arch::ISA::AVX2, T>(as<__m256i>(x)) + reduce_add_<arch::ISA::AVX2, T>(_mm512_extractf64x4_pd(as<__m512d>(x), 1));
        else if constexpr (epi16<T> || epu16<T>)
            return reduce_add_<arch::ISA::AVX2, T>(as<__m256i>(x)) + reduce_add_<arch::ISA::AVX2, T>(_mm512_extractf64x4_pd(as<__m512d>(x), 1));
    }
    else {
        return x;
    }
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline reduce_type<T> reduce_add_(V x, M mask) noexcept {
    return reduce_add_<ISA, T>(select_<ISA, T>(x, mask));
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline reduce_type<T> reduce_add_(V x, M mask, V src) noexcept {
    return reduce_add_<ISA, T>(select_<ISA, T>(x, src, mask));
}

__RAZE_VX_NAMESPACE_END
