#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <raze/memory/PointerToIntegral.h>
#include <raze/math/IntegralTypesConversions.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>
#include <src/raze/vx/storage/BestMaskChunk.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, intrin_type _Vector_, arithmetic_type _Type_>
struct _To_vector {
    template <raw_mask_type _Mask_>
    raze_nodiscard raze_always_inline auto operator()(_Mask_ __mask) const noexcept {
        constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
        constexpr auto __avx512dq = __has_avx512dq_support_v<_ISA_>;
        constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
        constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
        constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
        
        if constexpr (sizeof(_Vector_) == 16) {
            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return __as<_Vector_>(_mm_movm_epi64(__mask));
                    else return __as<_Vector_>(_mm512_maskz_mov_epi64(__mask, _All_ones<_ISA_, __m512i>()()));
                }
                else return __as<_Vector_>(_mm_cmpgt_epi32(_mm_and_si128(_mm_set1_epi8(
                    static_cast<i8>(__mask)), _mm_setr_epi32(1, 1, 2, 2)), _mm_setzero_si128()));
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return __as<_Vector_>(_mm_movm_epi32(__mask));
                    else return __as<_Vector_>(_mm512_maskz_mov_epi32(__mask, _All_ones<_ISA_, __m512i>()()));
                }
                else return __as<_Vector_>(_mm_cmpgt_epi32(_mm_and_si128(_mm_set1_epi8(
                    static_cast<i8>(__mask)), _mm_setr_epi32(1, 2, 4, 8)), _mm_setzero_si128()));
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__avx512vl && __avx512bw) return __as<_Vector_>(_mm_movm_epi16(__mask));
                else return __as<_Vector_>(_mm_cmpgt_epi16(_mm_and_si128(_mm_set1_epi8(static_cast<i8>(__mask)),
                    _mm_setr_epi32(0x00020001, 0x00080004, 0x00200010, 0x00800040)), _mm_setzero_si128()));
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__avx512vl && __avx512bw) return __as<_Vector_>(_mm_movm_epi8(__mask));
                else if constexpr (__ssse3) {
                    const auto __select = _mm_set1_epi64x(0x8040201008040201ull);
                    return __as<_Vector_>(_mm_cmpeq_epi8(_mm_and_si128(_mm_shuffle_epi8(
                        _mm_cvtsi32_si128(__mask), _mm_set_epi64x(0x0101010101010101ll, 0)), __select), __select));
                }
                else {
                    const auto __not_mask = u16(~__mask);

                    const auto __broadcasted_low_mask = _mm_set1_epi8(static_cast<i8>(__not_mask));
                    const auto __broadcasted_high_mask = _mm_set1_epi8(static_cast<i8>(__not_mask >> 8));

                    const auto __vector_mask_low = _mm_setr_epi32(0x08040201, 0x80402010, 0, 0);
                    const auto __vector_mask_high = _mm_setr_epi32(0, 0, 0x08040201, 0x80402010);

                    const auto __selected_low = _mm_and_si128(__broadcasted_low_mask, __vector_mask_low);
                    const auto __selected_high = _mm_and_si128(__broadcasted_high_mask, __vector_mask_high);

                    const auto __combined = _mm_or_si128(__selected_low, __selected_high);
                    return __as<_Vector_>(_mm_cmpeq_epi8(__combined, _mm_setzero_si128()));
                }
            }
        }
        else if constexpr (sizeof(_Vector_) == 32) {
            auto __fallback_avx = [=] () raze_always_inline_lambda {
                using _MaskType = __mmask_for_elements_t<0x20 / sizeof(_Type_)>;
                using _HalfType = IntegerForSize<__constexpr_max<(sizeof(_MaskType) >> 1), 1>()>::Unsigned;

                constexpr auto __maximum = math::__maximum_integral_limit<_HalfType>();
                constexpr auto __shift = (sizeof(_MaskType) << 2);

                const auto __low = _To_vector<arch::ISA::SSSE3, __m128i, _Type_>()(__mask & __maximum);
                const auto __high = _To_vector<arch::ISA::SSSE3, __m128i, _Type_>()(__mask >> __shift);

                return __as<_Vector_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
            };

            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return __as<_Vector_>(_mm256_movm_epi64(__mask));
                    else return __as<_Vector_>(_mm256_maskz_mov_epi64(__mask, _All_ones<_ISA_, __m256i>()()));
                }
                else if constexpr (__avx2) {
                    const auto __vector_mask = _mm256_set1_epi32(static_cast<i32>(__mask));
                    const auto __selected = _mm256_and_si256(__vector_mask, _mm256_setr_epi32(1, 0, 2, 0, 4, 0, 8, 0));

                    return __as<_Vector_>(_mm256_cmpgt_epi64(__selected, _mm256_setzero_si256()));
                }
                else return __fallback_avx();
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return __as<_Vector_>(_mm256_movm_epi32(__mask));
                    else return __as<_Vector_>(_mm256_maskz_mov_epi64(__mask, _All_ones<_ISA_, __m256i>()()));
                }
                else if constexpr(__avx2) {
                    const auto __vector_mask = _mm256_set1_epi32(static_cast<i32>(__mask));
                    const auto __selected = _mm256_and_si256(__vector_mask, _mm256_setr_epi32(1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80));

                    return __as<_Vector_>(_mm256_cmpgt_epi32(__selected, _mm256_setzero_si256()));
                }
                else return __fallback_avx();
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__avx512vl && __avx512bw) return __as<_Vector_>(_mm256_movm_epi16(__mask));
                else if constexpr (__avx2) {
                    const auto __vector_mask = _mm256_set1_epi16(static_cast<i16>(__mask));

                    const auto __shuffled = _mm256_shuffle_epi8(__vector_mask, _mm256_setr_epi32(0, 0, 0, 0, 0x00010001, 0x00010001, 0x00010001, 0x00010001));
                    const auto __select = _mm256_setr_epi32(0x00020001, 0x00080004, 0x00200010, 0x00800040, 0x00020001, 0x00080004, 0x00200010, 0x00800040);

                    return __as<_Vector_>(_mm256_cmpgt_epi16(_mm256_and_si256(__shuffled, __select), _mm256_setzero_si256()));
                }
                else return __fallback_avx();
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__avx512vl && __avx512bw) return __as<_Vector_>(_mm256_movm_epi8(__mask));
                else if constexpr (__avx2) {
                    const auto __vector_mask = _mm256_setr_epi32(__mask & 0xFFFF, 0, 0, 0, (u32(__mask) >> 16) & 0xFFFF, 0, 0, 0);

                    const auto __select = _mm256_set1_epi64x(0x8040201008040201ull);
                    const auto __shuffled = _mm256_shuffle_epi8(__vector_mask, _mm256_set_epi64x(0x0101010101010101ll, 0, 0x0101010101010101ll, 0));

                    return __as<_Vector_>(_mm256_cmpeq_epi8(_mm256_and_si256(__shuffled, __select), __select));
                }
                else return __fallback_avx();
            }
        }
        else if constexpr (sizeof(_Vector_) == 64) {
            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512dq) return __as<_Vector_>(_mm512_movm_epi64(__mask));
                else return __as<_Vector_>(_mm512_maskz_mov_epi64(__mask, _All_ones<_ISA_, __m512i>()()));
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512dq) return __as<_Vector_>(_mm512_movm_epi32(__mask));
                else return __as<_Vector_>(_mm512_maskz_mov_epi32(__mask, _All_ones<_ISA_, __m512i>()()));
            }
            else if constexpr (__avx512bw) {
                if constexpr (sizeof(_Type_) == 2) return __as<_Vector_>(_mm512_movm_epi16(__mask));
                else if constexpr (sizeof(_Type_) == 1) return __as<_Vector_>(_mm512_movm_epi8(__mask));
            }
            else {
                using _MaskType = __mmask_for_elements_t<0x40 / sizeof(_Type_)>;
                using _HalfType = IntegerForSize<__constexpr_max<(sizeof(_MaskType) >> 1), 1>()>::Unsigned;

                constexpr auto __maximum = math::__maximum_integral_limit<_HalfType>();
                constexpr auto __shift = (sizeof(_MaskType) << 2);

                const auto __low = _To_vector<arch::ISA::AVX2, __m256i, _Type_>()(__mask & __maximum);
                const auto __high = _To_vector<arch::ISA::AVX2, __m256i, _Type_>()((__mask >> __shift));

                return __as<_Vector_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
            }
        }
    }
};

__RAZE_VX_NAMESPACE_END
