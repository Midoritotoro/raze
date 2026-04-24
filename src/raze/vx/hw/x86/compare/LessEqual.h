#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/compare/Less.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	    _ISA_,
	arithmetic_type	_Type_>
struct _Less_equal {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline auto operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (__has_avx512vl_support_v<_ISA_>) {
                if constexpr (__is_pd_v<_Type_>) return _mm_cmp_pd_mask(__as<__m128d>(__x), __as<__m128d>(__y), _CMP_LE_OQ);
                else if constexpr (__is_ps_v<_Type_>) return _mm_cmp_ps_mask(__as<__m128>(__x), __as<__m128>(__y), _CMP_LE_OQ);
                else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm_cmple_epi64_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm_cmple_epi32_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                    if constexpr (__is_epi16_v<_Type_>) return _mm_cmple_epi16_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                    else if constexpr (__is_epu16_v<_Type_>) return _mm_cmple_epu16_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                    else if constexpr (__is_epi8_v<_Type_>) return _mm_cmple_epi8_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                    else if constexpr (__is_epu8_v<_Type_>) return _mm_cmple_epu8_mask(__as<__m128i>(__x), __as<__m128i>(__y));
                }
            }
            else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_cmple_ps(__as<__m128>(__x), __as<__m128>(__y)));
            else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_cmple_pd(__as<__m128d>(__x), __as<__m128d>(__y)));
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            if constexpr (__has_avx512vl_support_v<_ISA_>) {
                if constexpr (__is_pd_v<_Type_>) return _mm256_cmp_pd_mask(__as<__m256d>(__x), __as<__m256d>(__y), _CMP_LE_OQ);
                else if constexpr (__is_ps_v<_Type_>) return _mm256_cmp_ps_mask(__as<__m256>(__x), __as<__m256>(__y), _CMP_LE_OQ);
                else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm256_cmple_epi64_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm256_cmple_epi32_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                    if constexpr (__is_epi16_v<_Type_>) return _mm256_cmple_epi16_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                    else if constexpr (__is_epu16_v<_Type_>) return _mm256_cmple_epu16_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                    else if constexpr (__is_epi8_v<_Type_>) return _mm256_cmple_epi8_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                    else if constexpr (__is_epu8_v<_Type_>) return _mm256_cmple_epu8_mask(__as<__m256i>(__x), __as<__m256i>(__y));
                }
            }
            else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_cmp_ps(__as<__m128>(__x), __as<__m128>(__y), _CMP_LE_OQ));
            else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_cmp_pd(__as<__m128d>(__x), __as<__m128d>(__y), _CMP_LE_OQ));
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (__is_pd_v<_Type_>) return _mm512_cmple_pd_mask(__as<__m512d>(__x), __as<__m512d>(__y));
            else if constexpr (__is_ps_v<_Type_>) return _mm512_cmple_ps_mask(__as<__m512>(__x), __as<__m512>(__y));
            else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm512_cmple_epi64_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm512_cmple_epi32_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            else if constexpr (__has_avx512bw_support_v<_ISA_>) {
                if constexpr (__is_epi16_v<_Type_>) return _mm512_cmple_epi16_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epu16_v<_Type_>) return _mm512_cmple_epu16_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epi8_v<_Type_>) return _mm512_cmple_epi8_mask(__as<__m512i>(__x), __as<__m512i>(__y));
                else if constexpr (__is_epu8_v<_Type_>) return _mm512_cmple_epu8_mask(__as<__m512i>(__x), __as<__m512i>(__y));
            }
        }
        else {
            if constexpr (arithmetic_type<_Tp_>)
                return __x <= __y;
            else
                return _Not<_ISA_, _Type_>()(_Less<_ISA_, _Type_>()(__y, __x));
        }
	}
};

__RAZE_VX_NAMESPACE_END
