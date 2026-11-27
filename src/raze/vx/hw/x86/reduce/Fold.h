#pragma once 

#include <src/raze/traits/SimdTypeCheck.h>
#include <src/raze/vx/hw/x86/cast/As.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, class Reduce>
raze_always_inline T mirror_fold_(V x, Reduce reduce) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (sizeof(T) == 8) {
            if constexpr (pd<T>) {
                x = reduce(x, as<V>(_mm_unpackhi_pd(as<__m128d>(x), as<__m128d>(x))));
                return _mm_cvtsd_f64(as<__m128d>(x));
            }
            else {
                x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 8)));
                return _mm_cvtsi128_si64(as<__m128i>(x));
            }
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (ps<T>) {
                x = reduce(x, as<V>(_mm_movehl_ps(as<__m128>(x), as<__m128>(x))));
                x = reduce(x, as<V>(_mm_shuffle_ps(as<__m128>(x), as<__m128>(x), 1)));
                return _mm_cvtss_f32(as<__m128>(x));
            }
            else {
                x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 8)));
                x = reduce(x, as<V>(_mm_srli_si128(as<__m128i>(x), 4)));
                return _mm_cvtsi128_si32(as<__m128i>(x));
            }
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
        using Low = traits::deduce_simd_vector_type<T, 128>;
        Low hi;
        if constexpr (pd<T>) hi = _mm256_extractf128_pd(as<__m256d>(x), 1);
        else if constexpr (ps<T>) hi = _mm256_extractf128_ps(as<__m256>(x), 1);
        else if constexpr (has_avx2<ISA>) hi = as<Low>(_mm256_extracti128_si256(as<__m256i>(x), 1));
        else hi = as<Low>(_mm256_extractf128_si256(as<__m256i>(x), 1));

        return mirror_fold_<ISA, T>(reduce(as<Low>(x), hi), reduce);
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (sizeof(T) == 8 && !pd<T> && !has_avx512vl<ISA>) {
            x = reduce(x, as<V>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0x4E)));
            x = reduce(x, as<V>(_mm512_permutex_epi64(as<__m512i>(x), 0x4E)));
            x = reduce(x, as<V>(_mm512_permutex_epi64(as<__m512i>(x), 0xB1)));
            return static_cast<T>(_mm_cvtsi128_si64(as<__m128i>(x)));
        }
        else {
            using Low = traits::deduce_simd_vector_type<T, 256>;
            Low hi;
            if constexpr (pd<T>) hi = _mm512_extractf64x4_pd(as<__m512d>(x), 1);
            else if constexpr (ps<T>) hi = as<Low>(_mm512_extractf64x4_pd(as<__m512d>(x), 1));
            else hi = as<Low>(_mm512_extracti64x4_epi64(as<__m512i>(x), 1));

            return mirror_fold_<ISA, T>(reduce(as<Low>(x), hi), reduce);
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
