#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	    _ISA_,
	arithmetic_type	_Type_>
struct _Equal {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline auto operator()(_Tp_ __x, _Tp_ __y) const noexcept {
        constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
        constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
        constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
        constexpr auto __sse41 = __has_sse41_support_v<_ISA_>;

        if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (__is_pd_v<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmp_pd_mask(__as<__m128d>(__x), __as<__m128d>(__y), _CMP_EQ_OQ);
                else return __as<_Tp_>(_mm_cmpeq_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
            }
            else if constexpr (__is_ps_v<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmp_ps_mask(__as<__m128>(__x), __as<__m128>(__y), _CMP_EQ_OQ);
                else return __as<_Tp_>(_mm_cmpeq_ps(__as<__m128>(__x), __as<__m128>(__y)));
            }
            else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmpeq_epi64_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__sse41) return __as<_Tp_>(_mm_cmpeq_epi64(__as<__m128i>(__x), __as<__m128i>(__y)));
                else {
                    const auto __equal_mask = _mm_cmpeq_epi32(__as<__m128i>(__x), __as<__m128i>(__y));
                    return __as<_Tp_>(_mm_and_si128(__equal_mask, _mm_shuffle_epi32(__equal_mask, 0xB1)));
                }
            }
            else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmpeq_epi32_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else return __as<_Tp_>(_mm_cmpeq_epi32(__as<__m128i>(__x), __as<__m128i>(__y)));
            }
            else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm_cmpeq_epi16_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else return __as<_Tp_>(_mm_cmpeq_epi16(__as<__m128i>(__x), __as<__m128i>(__y)));
            }
            else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm_cmpeq_epi8_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else return __as<_Tp_>(_mm_cmpeq_epi8(__as<__m128i>(__x), __as<__m128i>(__y)));
            }
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            auto __fallback_avx = [=]() {
                const auto __low = _Equal<arch::ISA::SSE42, _Type_>()(__as<__m128i>(__x), __as<__m128i>(__y));
                const auto __high = _Equal<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1),
                    _mm256_extractf128_si256(__as<__m256i>(__y), 1));

                return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__low), __high, 1));
            };

            if constexpr (__is_pd_v<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmp_pd_mask(__as<__m256d>(__x), __as<__m256d>(__y), _CMP_EQ_OQ);
                else return __as<_Tp_>(_mm256_cmp_pd(__as<__m256d>(__x), __as<__m256d>(__y), _CMP_EQ_OQ));
            }
            else if constexpr (__is_ps_v<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmp_ps_mask(__as<__m256>(__x), __as<__m256>(__y), _CMP_EQ_OQ);
                else return __as<_Tp_>(_mm256_cmp_ps(__as<__m256>(__x), __as<__m256>(__y), _CMP_EQ_OQ));
            }
            else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmpeq_epi64_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__avx2) return __as<_Tp_>(_mm256_cmpeq_epi64(__as<__m256i>(__x), __as<__m256i>(__y)));
                else return __fallback_avx();
            }
            else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmpeq_epi32_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__avx2) return __as<_Tp_>(_mm256_cmpeq_epi32(__as<__m256i>(__x), __as<__m256i>(__y)));
                else return __fallback_avx();
            }
            else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm256_cmpeq_epi16_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__avx2) return __as<_Tp_>(_mm256_cmpeq_epi16(__as<__m256i>(__x), __as<__m256i>(__y)));
                else return __fallback_avx();
            }
            else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
                if constexpr (__avx512vl && __avx512bw) return _mm256_cmpeq_epi8_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__avx2) return __as<_Tp_>(_mm256_cmpeq_epi8(__as<__m256i>(__x), __as<__m256i>(__y)));
                else return __fallback_avx();
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (__is_pd_v<_Type_>) return _mm512_cmpeq_pd_mask(__as<__m512d>(__x), __as<__m512d>(__y));
            else if constexpr (__is_ps_v<_Type_>) return _mm512_cmpeq_ps_mask(__as<__m512>(__x), __as<__m512>(__y));
            else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm512_cmpeq_epi64_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm512_cmpeq_epi32_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                if constexpr (__is_epi16_v<_Type_>) return _mm512_cmpeq_epi16_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epu16_v<_Type_>) return _mm512_cmpeq_epu16_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epi8_v<_Type_>) return _mm512_cmpeq_epi8_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epu8_v<_Type_>) return _mm512_cmpeq_epu8_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            }
            else {
                const auto __low = _Equal<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x), __as<__m256i>(__y));
                const auto __high = _Equal<arch::ISA::AVX2, _Type_>()(_mm512_extracti64x4_epi64(__as<__m512i>(__x), 1),
                    _mm512_extracti64x4_epi64(__as<__m512i>(__y), 1));

                return __as<_Tp_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
            }
        }
        else {
            return __x == __y;
        }
	}
};

__RAZE_VX_NAMESPACE_END
