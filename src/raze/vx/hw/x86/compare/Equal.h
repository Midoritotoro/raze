#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Equal {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline auto operator()(_Tp_ __x, _Tp_ __y) const noexcept {
        constexpr auto __avx512vl = has_avx512vl<_ISA_>;
        constexpr auto __avx512bw = has_avx512bw<_ISA_>;
        constexpr auto __avx2 = has_avx2<_ISA_>;
        constexpr auto __sse41 = has_sse41<_ISA_>;

        if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (pd<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmp_pd_mask(as<__m128d>(__x), as<__m128d>(__y), _CMP_EQ_OQ);
                else return as<_Tp_>(_mm_cmpeq_pd(as<__m128d>(__x), as<__m128d>(__y)));
            }
            else if constexpr (ps<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmp_ps_mask(as<__m128>(__x), as<__m128>(__y), _CMP_EQ_OQ);
                else return as<_Tp_>(_mm_cmpeq_ps(as<__m128>(__x), as<__m128>(__y)));
            }
            else if constexpr (epi64<_Type_> || epu64<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmpeq_epi64_mask(as<__m128i>(__x), as<__m128i>(__y));
                else if constexpr (__sse41) return as<_Tp_>(_mm_cmpeq_epi64(as<__m128i>(__x), as<__m128i>(__y)));
                else {
                    const auto __equal_mask = _mm_cmpeq_epi32(as<__m128i>(__x), as<__m128i>(__y));
                    return as<_Tp_>(_mm_and_si128(__equal_mask, _mm_shuffle_epi32(__equal_mask, 0xB1)));
                }
            }
            else if constexpr (epi32<_Type_> || epu32<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmpeq_epi32_mask(as<__m128i>(__x), as<__m128i>(__y));
                else return as<_Tp_>(_mm_cmpeq_epi32(as<__m128i>(__x), as<__m128i>(__y)));
            }
            else if constexpr (epi16<_Type_> || epu16<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm_cmpeq_epi16_mask(as<__m128i>(__x), as<__m128i>(__y));
                else return as<_Tp_>(_mm_cmpeq_epi16(as<__m128i>(__x), as<__m128i>(__y)));
            }
            else if constexpr (epi8<_Type_> || epu8<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm_cmpeq_epi8_mask(as<__m128i>(__x), as<__m128i>(__y));
                else return as<_Tp_>(_mm_cmpeq_epi8(as<__m128i>(__x), as<__m128i>(__y)));
            }
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            auto __fallback_avx = [=]() {
                const auto __low = _Equal<arch::ISA::SSE42, _Type_>()(as<__m128i>(__x), as<__m128i>(__y));
                const auto __high = _Equal<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(as<__m256i>(__x), 1),
                    _mm256_extractf128_si256(as<__m256i>(__y), 1));

                return as<_Tp_>(_mm256_insertf128_si256(as<__m256i>(__low), __high, 1));
            };

            if constexpr (pd<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmp_pd_mask(as<__m256d>(__x), as<__m256d>(__y), _CMP_EQ_OQ);
                else return as<_Tp_>(_mm256_cmp_pd(as<__m256d>(__x), as<__m256d>(__y), _CMP_EQ_OQ));
            }
            else if constexpr (ps<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmp_ps_mask(as<__m256>(__x), as<__m256>(__y), _CMP_EQ_OQ);
                else return as<_Tp_>(_mm256_cmp_ps(as<__m256>(__x), as<__m256>(__y), _CMP_EQ_OQ));
            }
            else if constexpr (epi64<_Type_> || epu64<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmpeq_epi64_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (__avx2) return as<_Tp_>(_mm256_cmpeq_epi64(as<__m256i>(__x), as<__m256i>(__y)));
                else return __fallback_avx();
            }
            else if constexpr (epi32<_Type_> || epu32<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmpeq_epi32_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (__avx2) return as<_Tp_>(_mm256_cmpeq_epi32(as<__m256i>(__x), as<__m256i>(__y)));
                else return __fallback_avx();
            }
            else if constexpr (epi16<_Type_> || epu16<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm256_cmpeq_epi16_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (__avx2) return as<_Tp_>(_mm256_cmpeq_epi16(as<__m256i>(__x), as<__m256i>(__y)));
                else return __fallback_avx();
            }
            else if constexpr (epi8<_Type_> || epu8<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm256_cmpeq_epi8_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (__avx2) return as<_Tp_>(_mm256_cmpeq_epi8(as<__m256i>(__x), as<__m256i>(__y)));
                else return __fallback_avx();
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (pd<_Type_>) return _mm512_cmpeq_pd_mask(as<__m512d>(__x), as<__m512d>(__y));
            else if constexpr (ps<_Type_>) return _mm512_cmpeq_ps_mask(as<__m512>(__x), as<__m512>(__y));
            else if constexpr (epi64<_Type_> || epu64<_Type_>) return _mm512_cmpeq_epi64_mask(as<__m512i>(__x), as<__m512i>(__y));
            else if constexpr (epi32<_Type_> || epu32<_Type_>) return _mm512_cmpeq_epi32_mask(as<__m512i>(__x), as<__m512i>(__y));
            else if constexpr (has_avx512bw<_ISA_>) {
                if constexpr (epi16<_Type_>) return _mm512_cmpeq_epi16_mask(as<__m512i>(__x), as<__m512i>(__y));
                else if constexpr (epu16<_Type_>) return _mm512_cmpeq_epu16_mask(as<__m512i>(__x), as<__m512i>(__y));
                else if constexpr (epi8<_Type_>) return _mm512_cmpeq_epi8_mask(as<__m512i>(__x), as<__m512i>(__y));
                else if constexpr (epu8<_Type_>) return _mm512_cmpeq_epu8_mask(as<__m512i>(__x), as<__m512i>(__y));
            }
            else {
                const auto __low = _Equal<arch::ISA::AVX2, _Type_>()(as<__m256i>(__x), as<__m256i>(__y));
                const auto __high = _Equal<arch::ISA::AVX2, _Type_>()(_mm512_extracti64x4_epi64(as<__m512i>(__x), 1),
                    _mm512_extracti64x4_epi64(as<__m512i>(__y), 1));

                return as<_Tp_>(_mm512_inserti64x4(as<__m512i>(__low), __high, 1));
            }
        }
        else {
            return __x == __y;
        }
	}
};

__RAZE_VX_NAMESPACE_END
