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
            const auto lo = static_cast<u64>(static_cast<u32>(_mm_cvtsi128_si32(as<__m128i>(x)))) |
                (static_cast<u64>(static_cast<u32>(extract_<arch::ISA::SSE2, i32>(x, 1))) << 32);
            const auto hi = static_cast<u64>(static_cast<u32>(extract_<arch::ISA::SSE2, i32>(x, 2))) |
                (static_cast<u64>(static_cast<u32>(extract_<arch::ISA::SSE2, i32>(x, 3))) << 32);
            return static_cast<ReduceType>(lo + hi);
#else 
            const auto lo = static_cast<u64>(_mm_cvtsi128_si64(as<__m128i>(x)));
            const auto hi = static_cast<u64>(_mm_cvtsi128_si64(_mm_unpackhi_epi64(as<__m128i>(x), as<__m128i>(x))));
            return static_cast<ReduceType>(lo + hi);
#endif // defined(raze_processor_x86_32)
        }
        else if constexpr (epi32<T>) {
            if constexpr (has_sse41<ISA>) {
                const auto lo64 = _mm_cvtepi32_epi64(as<__m128i>(x));
                const auto hi64 = _mm_cvtepi32_epi64(_mm_unpackhi_epi64(as<__m128i>(x), as<__m128i>(x)));
                return reduce_add_<ISA, i64>(_mm_add_epi64(lo64, hi64));
            }
            else {
                const auto sign = _mm_srai_epi32(as<__m128i>(x), 31);
                const auto lo64 = _mm_unpacklo_epi32(as<__m128i>(x), sign);
                const auto hi64 = _mm_unpackhi_epi32(as<__m128i>(x), sign);
                return reduce_add_<ISA, i64>(_mm_add_epi64(lo64, hi64));
            }
        }
        else if constexpr (epu32<T>) {
            const auto zeros = _mm_setzero_si128();
            const auto lo64 = _mm_unpacklo_epi32(as<__m128i>(x), zeros);
            const auto hi64 = _mm_unpackhi_epi32(as<__m128i>(x), zeros);
            return reduce_add_<ISA, u64>(_mm_add_epi64(lo64, hi64));
        }
        else if constexpr (epi16<T>) {
            const auto pairs32 = _mm_madd_epi16(as<__m128i>(x), _mm_set1_epi16(1));
            const auto shuf1 = _mm_shuffle_epi32(pairs32, _MM_SHUFFLE(2, 3, 0, 1));
            const auto sum2 = _mm_add_epi32(pairs32, shuf1);
            const auto shuf2 = _mm_shuffle_epi32(sum2, _MM_SHUFFLE(1, 0, 3, 2));
            return static_cast<ReduceType>(_mm_cvtsi128_si32(_mm_add_epi32(sum2, shuf2)));
        }
        else if constexpr (epu16<T>) {
            const auto zeros = _mm_setzero_si128();
            const auto low = _mm_unpacklo_epi16(as<__m128i>(x), zeros);
            const auto high = _mm_unpackhi_epi16(as<__m128i>(x), zeros);
            const auto reduce1 = _mm_add_epi32(low, high);
            const auto shuffle1 = _mm_shuffle_epi32(reduce1, _MM_SHUFFLE(2, 3, 0, 1));
            const auto reduce2 = _mm_add_epi32(reduce1, shuffle1);
            const auto shuffled2 = _mm_shuffle_epi32(reduce2, _MM_SHUFFLE(1, 0, 3, 2));
            return static_cast<ReduceType>(static_cast<u32>(_mm_cvtsi128_si32(_mm_add_epi32(reduce2, shuffled2))));
        }
        else if constexpr (epi8<T>) {
            const auto biased = _mm_xor_si128(as<__m128i>(x), _mm_set1_epi8(static_cast<i8>(-128)));
            const auto sad = _mm_sad_epu8(biased, _mm_setzero_si128());
            return static_cast<ReduceType>(reduce_add_<ISA, u64>(sad)) - static_cast<ReduceType>(16 * 128);
        }
        else if constexpr (epu8<T>) {
            const auto sad = _mm_sad_epu8(as<__m128i>(x), _mm_setzero_si128());
            return reduce_add_<ISA, u64>(sad);
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
                return static_cast<ReduceType>(_mm_cvtss_f32(_mm_add_ss(reduce1, shuffled2)));
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
            else if constexpr (epi32<T>) {
                const auto lo64 = _mm256_cvtepi32_epi64(as<__m128i>(x));
                const auto hi64 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(as<__m256i>(x), 1));
                return reduce_add_<ISA, i64>(_mm256_add_epi64(lo64, hi64));
            }
            else if constexpr (epu32<T>) {
                const auto lo64 = _mm256_cvtepu32_epi64(as<__m128i>(x));
                const auto hi64 = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(as<__m256i>(x), 1));
                return reduce_add_<ISA, u64>(_mm256_add_epi64(lo64, hi64));
            }
            else if constexpr (epi16<T>) {
                const auto pairs32 = _mm256_madd_epi16(as<__m256i>(x), _mm256_set1_epi16(1));
                const auto sum4 = _mm_add_epi32(_mm256_castsi256_si128(pairs32), _mm256_extracti128_si256(pairs32, 1));
                const auto shuf1 = _mm_shuffle_epi32(sum4, _MM_SHUFFLE(2, 3, 0, 1));
                const auto sum2 = _mm_add_epi32(sum4, shuf1);
                const auto shuf2 = _mm_shuffle_epi32(sum2, _MM_SHUFFLE(1, 0, 3, 2));
                return static_cast<ReduceType>(_mm_cvtsi128_si32(_mm_add_epi32(sum2, shuf2)));
            }
            else if constexpr (epu16<T>) {
                const auto zeros = _mm256_setzero_si256();
                const auto lo32 = _mm256_unpacklo_epi16(as<__m256i>(x), zeros);
                const auto hi32 = _mm256_unpackhi_epi16(as<__m256i>(x), zeros);
                const auto sum8 = _mm256_add_epi32(lo32, hi32);
                const auto sum4 = _mm_add_epi32(_mm256_castsi256_si128(sum8), _mm256_extracti128_si256(sum8, 1));
                const auto shuf1 = _mm_shuffle_epi32(sum4, _MM_SHUFFLE(2, 3, 0, 1));
                const auto sum2 = _mm_add_epi32(sum4, shuf1);
                const auto shuf2 = _mm_shuffle_epi32(sum2, _MM_SHUFFLE(1, 0, 3, 2));
                return static_cast<ReduceType>(static_cast<u32>(_mm_cvtsi128_si32(_mm_add_epi32(sum2, shuf2))));
            }
            else if constexpr (epi8<T>) {
                const auto biased = _mm256_xor_si256(as<__m256i>(x), _mm256_set1_epi8(static_cast<i8>(-128)));
                const auto reduce1 = _mm256_sad_epu8(biased, _mm256_setzero_si256());

                const auto low64 = _mm256_castsi256_si128(reduce1);
                const auto high64 = _mm256_extracti128_si256(reduce1, 1);

                return static_cast<ReduceType>(reduce_add_<arch::ISA::SSSE3, u64>(_mm_add_epi64(low64, high64)))
                    - static_cast<ReduceType>(32 * 128);
            }
            else if constexpr (epu8<T>) {
                const auto reduce1 = _mm256_sad_epu8(as<__m256i>(x), _mm256_setzero_si256());

                const auto low64 = _mm256_castsi256_si128(reduce1);
                const auto high64 = _mm256_extracti128_si256(reduce1, 1);

                return reduce_add_<arch::ISA::SSSE3, u64>(_mm_add_epi64(low64, high64));
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
            if constexpr (pd<T>) {
                const auto low = as<__m128d>(x);
                const auto high = _mm256_extractf128_pd(as<__m256d>(x), 1);
                return reduce_add_<arch::ISA::SSE42, T>(_mm_add_pd(low, high));
            }
            else if constexpr (ps<T>) {
                const auto low = as<__m128>(x);
                const auto high = _mm256_extractf128_ps(as<__m256>(x), 1);
                return reduce_add_<arch::ISA::SSE42, T>(_mm_add_ps(low, high));
            }
            else {
                const auto low = as<__m128i>(x);
                const auto high = _mm256_extractf128_si256(as<__m256i>(x), 1);
                return reduce_add_<arch::ISA::SSE42, T>(low) + reduce_add_<arch::ISA::SSE42, T>(high);
            }
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (epi64<T> || epu64<T>) return static_cast<ReduceType>(_mm512_reduce_add_epi64(as<__m512i>(x)));
        else if constexpr (epi32<T>) {
            const auto lo64 = _mm512_cvtepi32_epi64(as<__m256i>(x));
            const auto hi64 = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(as<__m512i>(x), 1));
            return static_cast<ReduceType>(_mm512_reduce_add_epi64(_mm512_add_epi64(lo64, hi64)));
        }
        else if constexpr (epu32<T>) {
            const auto lo64 = _mm512_cvtepu32_epi64(as<__m256i>(x));
            const auto hi64 = _mm512_cvtepu32_epi64(_mm512_extracti64x4_epi64(as<__m512i>(x), 1));
            return static_cast<ReduceType>(_mm512_reduce_add_epi64(_mm512_add_epi64(lo64, hi64)));
        }
        else if constexpr (ps<T>) return static_cast<ReduceType>(_mm512_reduce_add_ps(as<__m512>(x)));
        else if constexpr (pd<T>) return static_cast<ReduceType>(_mm512_reduce_add_pd(as<__m512d>(x)));
        else if constexpr (epi8<T> || epu8<T> || epi16<T> || epu16<T>) {
            return reduce_add_<arch::ISA::AVX2, T>(as<__m256i>(x))
                + reduce_add_<arch::ISA::AVX2, T>(_mm512_extracti64x4_epi64(as<__m512i>(x), 1));
        }
    }
    else {
        return static_cast<ReduceType>(x);
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
