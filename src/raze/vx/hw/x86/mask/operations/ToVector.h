#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <raze/math/BitCast.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>
#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_type V, arithmetic_type T, raw_mask_type M>
raze_always_inline auto to_vector_(M mask) noexcept {
    if constexpr (intrin_type<M>)
        return as<V>(mask);
    else if constexpr (sizeof(V) == 16) {
        if constexpr (sizeof(T) == 8) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return as<V>(_mm_movm_epi64(mask));
                else return as<V>(_mm512_maskz_mov_epi64(mask, all_ones_<ISA, __m512i>()));
            }
            else {
                const auto broadcasted = _mm_set1_epi8(static_cast<i8>(mask));
                const auto intersection = _mm_and_si128(broadcasted, _mm_setr_epi32(1, 1, 2, 2));
                return as<V>(_mm_cmpgt_epi32(intersection, _mm_setzero_si128()));
            }
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return as<V>(_mm_movm_epi32(mask));
                else return as<V>(_mm512_maskz_mov_epi32(mask, all_ones_<ISA, __m512i>()));
            }
            else return as<V>(_mm_cmpgt_epi32(_mm_and_si128(_mm_set1_epi8(
                static_cast<i8>(mask)), _mm_setr_epi32(1, 2, 4, 8)), _mm_setzero_si128()));
        }
        else if constexpr (sizeof(T) == 2) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return as<V>(_mm_movm_epi16(mask));
            else return as<V>(_mm_cmpgt_epi16(_mm_and_si128(_mm_set1_epi8(static_cast<i8>(mask)),
                _mm_setr_epi32(0x00020001, 0x00080004, 0x00200010, 0x00800040)), _mm_setzero_si128()));
        }
        else if constexpr (sizeof(T) == 1) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return as<V>(_mm_movm_epi8(mask));
            else if constexpr (has_ssse3<ISA>) {
                const auto select = _mm_set1_epi64x(0x8040201008040201ull);
                return as<V>(_mm_cmpeq_epi8(_mm_and_si128(_mm_shuffle_epi8(
                    _mm_cvtsi32_si128(mask), _mm_set_epi64x(0x0101010101010101ll, 0)), select), select));
            }
            else {
                const auto not_mask = u16(~mask);

                const auto broadcasted_low_mask = _mm_set1_epi8(static_cast<i8>(not_mask));
                const auto broadcasted_high_mask = _mm_set1_epi8(static_cast<i8>(not_mask >> 8));

                const auto vector_mask_low = _mm_setr_epi32(0x08040201, 0x80402010, 0, 0);
                const auto vector_mask_high = _mm_setr_epi32(0, 0, 0x08040201, 0x80402010);

                const auto selected_low = _mm_and_si128(broadcasted_low_mask, vector_mask_low);
                const auto selected_high = _mm_and_si128(broadcasted_high_mask, vector_mask_high);

                const auto combined = _mm_or_si128(selected_low, selected_high);
                return as<V>(_mm_cmpeq_epi8(combined, _mm_setzero_si128()));
            }
        }
    }
    else if constexpr (sizeof(V) == 32) {
        auto fallback_avx = [=] () raze_always_inline_lambda {
            using MaskType = mmask_for_elements_t<0x20 / sizeof(T)>;

            constexpr auto half_bits = (sizeof(V) / 2) / sizeof(T);
            constexpr MaskType low_mask = (MaskType(1) << half_bits) - 1;

            const auto low = to_vector_<arch::ISA::SSSE3, __m128i, T>(MaskType(mask & low_mask));
            const auto high = to_vector_<arch::ISA::SSSE3, __m128i, T>(MaskType(mask >> half_bits));

            return as<V>(_mm256_insertf128_si256(as<__m256i>(low), as<__m128i>(high), 1));
        };

        if constexpr (sizeof(T) == 8) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return as<V>(_mm256_movm_epi64(mask));
                else return as<V>(_mm256_maskz_mov_epi64(mask, all_ones_<ISA, __m256i>()));
            }
            else if constexpr (has_avx2<ISA>) {
                const auto vector_mask = _mm256_set1_epi32(static_cast<i32>(mask));
                const auto selected = _mm256_and_si256(vector_mask, _mm256_setr_epi32(1, 0, 2, 0, 4, 0, 8, 0));

                return as<V>(_mm256_cmpgt_epi64(selected, _mm256_setzero_si256()));
            }
            else return fallback_avx();
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (has_avx512vl<ISA>) {
                if constexpr (has_avx512dq<ISA>) return as<V>(_mm256_movm_epi32(mask));
                else return as<V>(_mm256_maskz_mov_epi64(mask, all_ones_<ISA, __m256i>()));
            }
            else if constexpr(has_avx2<ISA>) {
                const auto vector_mask = _mm256_set1_epi32(static_cast<i32>(mask));
                const auto selected = _mm256_and_si256(vector_mask, _mm256_setr_epi32(1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80));

                return as<V>(_mm256_cmpgt_epi32(selected, _mm256_setzero_si256()));
            }
            else return fallback_avx();
        }
        else if constexpr (sizeof(T) == 2) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return as<V>(_mm256_movm_epi16(mask));
            else if constexpr (has_avx2<ISA>) {
                const auto vector_mask = _mm256_set1_epi16(static_cast<i16>(mask));

                const auto shuffled = _mm256_shuffle_epi8(vector_mask, _mm256_setr_epi32(0, 0, 0, 0, 0x00010001, 0x00010001, 0x00010001, 0x00010001));
                const auto select = _mm256_setr_epi32(0x00020001, 0x00080004, 0x00200010, 0x00800040, 0x00020001, 0x00080004, 0x00200010, 0x00800040);

                return as<V>(_mm256_cmpgt_epi16(_mm256_and_si256(shuffled, select), _mm256_setzero_si256()));
            }
            else return fallback_avx();
        }
        else if constexpr (sizeof(T) == 1) {
            if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return as<V>(_mm256_movm_epi8(mask));
            else if constexpr (has_avx2<ISA>) {
                const auto vector_mask = _mm256_setr_epi32(mask & 0xFFFF, 0, 0, 0, (u32(mask) >> 16) & 0xFFFF, 0, 0, 0);

                const auto select = _mm256_set1_epi64x(0x8040201008040201ull);
                const auto shuffled = _mm256_shuffle_epi8(vector_mask, _mm256_set_epi64x(0x0101010101010101ll, 0, 0x0101010101010101ll, 0));

                return as<V>(_mm256_cmpeq_epi8(_mm256_and_si256(shuffled, select), select));
            }
            else return fallback_avx();
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (sizeof(T) == 8) {
            if constexpr (has_avx512dq<ISA>) return as<V>(_mm512_movm_epi64(mask));
            else return as<V>(_mm512_maskz_mov_epi64(mask, all_ones_<ISA, __m512i>()));
        }
        else if constexpr (sizeof(T) == 4) {
            if constexpr (has_avx512dq<ISA>) return as<V>(_mm512_movm_epi32(mask));
            else return as<V>(_mm512_maskz_mov_epi32(mask, all_ones_<ISA, __m512i>()));
        }
        else if constexpr (has_avx512bw<ISA>) {
            if constexpr (sizeof(T) == 2) return as<V>(_mm512_movm_epi16(mask));
            else if constexpr (sizeof(T) == 1) return as<V>(_mm512_movm_epi8(mask));
        }
        else {
            using MaskType = mmask_for_elements_t<0x40 / sizeof(T)>;

            constexpr auto half_bits = (sizeof(V) / 2) / sizeof(T);
            constexpr MaskType low_mask = (MaskType(1) << half_bits) - 1;

            const auto low = to_vector_<arch::ISA::AVX2, __m256i, T>(MaskType(mask & low_mask));
            const auto high = to_vector_<arch::ISA::AVX2, __m256i, T>(MaskType(mask >> half_bits));

            return as<V>(_mm512_inserti64x4(as<__m512i>(low), as<__m256i>(high), 1));
        }
    }
}

__RAZE_VX_NAMESPACE_END
