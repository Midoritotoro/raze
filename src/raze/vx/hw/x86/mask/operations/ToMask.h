#pragma once 

#include <raze/math/BitCast.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/vx/storage/BestMaskChunk.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, raw_mask_type M>
raze_nodiscard raze_always_inline auto to_mask_(M x) noexcept {
    using MaskType = mmask_for_elements_t<sizeof(M) / sizeof(T)>;

    if constexpr (std::is_integral_v<M>) { return x; }
	else if constexpr (sizeof(M) == 16) {
        if constexpr (sizeof(T) == 8) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return static_cast<MaskType>(_mm_movepi64_mask(as<__m128i>(x)));
                else return static_cast<MaskType>(_mm_cmplt_epi64_mask(as<__m128i>(x), _mm_setzero_si128()));
            }
            else return static_cast<MaskType>(_mm_movemask_pd(as<__m128d>(x)));
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return static_cast<MaskType>(_mm_movepi32_mask(as<__m128i>(x)));
                else return static_cast<MaskType>(_mm_cmplt_epi32_mask(as<__m128i>(x), _mm_setzero_si128()));
            }
            else return static_cast<MaskType>(_mm_movemask_ps(as<__m128>(x)));
        }
        else if constexpr (sizeof(T) == 2) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return static_cast<MaskType>(_mm_movepi16_mask(as<__m128i>(x)));
            else return static_cast<MaskType>(_mm_movemask_epi8(_mm_packs_epi16(as<__m128i>(x), _mm_setzero_si128())));
        }
        else if constexpr (sizeof(T) == 1) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return static_cast<MaskType>(_mm_movepi8_mask(as<__m128i>(x)));
            return static_cast<MaskType>(_mm_movemask_epi8(as<__m128i>(x)));
        }
	}
    else if constexpr (sizeof(M) == 32) {
        auto fallback_avx = [&] () raze_always_inline_lambda {
            constexpr auto half_bits = (sizeof(M) / sizeof(T)) >> 1;

            const auto low = to_mask_<arch::ISA::SSE42, T>(as<__m128i>(x));
            const auto high = to_mask_<arch::ISA::SSE42, T>(_mm256_extractf128_si256(as<__m256i>(x), 1));

            return ((static_cast<MaskType>(high) << half_bits) | static_cast<MaskType>(low));
        };

        if constexpr (sizeof(T) == 8) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return static_cast<MaskType>(_mm256_movepi64_mask(as<__m256i>(x)));
                else return static_cast<MaskType>(_mm256_cmplt_epi64_mask(as<__m256i>(x), _mm256_setzero_si256()));
            }
            else return static_cast<MaskType>(_mm256_movemask_pd(as<__m256d>(x)));
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return static_cast<MaskType>(_mm256_movepi32_mask(as<__m256i>(x)));
                else return static_cast<MaskType>(_mm256_cmplt_epi32_mask(as<__m256i>(x), _mm256_setzero_si256()));
            }
            else return static_cast<MaskType>(_mm256_movemask_ps(as<__m256>(x)));
        }
        else if constexpr (sizeof(T) == 2) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return static_cast<MaskType>(_mm256_movepi16_mask(as<__m256i>(x)));
            else if constexpr (has_avx2<ISA>) return static_cast<MaskType>(_mm256_movemask_epi8(_mm256_permute4x64_epi64(_mm256_packs_epi16(as<__m256i>(x), _mm256_setzero_si256()), 0xD8)));
            else return static_cast<MaskType>(fallback_avx());
        }
        else if constexpr (sizeof(T) == 1) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return static_cast<MaskType>(_mm256_movepi8_mask(as<__m256i>(x)));
            else if constexpr (has_avx2<ISA>) return static_cast<MaskType>(_mm256_movemask_epi8(as<__m256i>(x)));
            else return static_cast<MaskType>(fallback_avx());
        }
    }
    else if constexpr (sizeof(M) == 64) {
        auto fallback_avx512f = [&]() raze_always_inline_lambda {
            constexpr auto half_bits = (sizeof(M) / sizeof(T)) >> 1;

            const auto low = to_mask_<arch::ISA::AVX2, T>(as<__m256i>(x));
            const auto high = to_mask_<arch::ISA::AVX2, T>(_mm512_extractf64x4_pd(as<__m512d>(x), 1));

            return ((static_cast<MaskType>(high) << half_bits) | static_cast<MaskType>(low));
        };

        if constexpr (sizeof(T) == 8) {
            if constexpr (has_avx512dq<ISA>) return static_cast<MaskType>(_mm512_movepi64_mask(as<__m512i>(x)));
            else return static_cast<MaskType>(_mm512_cmplt_epi64_mask(as<__m512i>(x), _mm512_setzero_si512()));
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (has_avx512dq<ISA>) return static_cast<MaskType>(_mm512_movepi32_mask(as<__m512i>(x)));
            else return static_cast<MaskType>(_mm512_cmplt_epi32_mask(as<__m512i>(x), _mm512_setzero_si512()));
        }
        else if constexpr (sizeof(T) == 2) {
            if constexpr (has_avx512bw<ISA>) return static_cast<MaskType>(_mm512_movepi16_mask(as<__m512i>(x)));
            else return static_cast<MaskType>(fallback_avx512f());
        }
        else if constexpr (sizeof(T) == 1) {
            if constexpr (has_avx512bw<ISA>) return static_cast<MaskType>(_mm512_movepi8_mask(as<__m512i>(x)));
            else return static_cast<MaskType>(fallback_avx512f());
        }
    }
}


__RAZE_VX_NAMESPACE_END
