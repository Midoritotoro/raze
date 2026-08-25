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
        constexpr auto __avx512bw = has_avx512bw<_ISA_>;
        constexpr auto __avx512vl = has_avx512vl<_ISA_>;
        constexpr auto __sse41 = has_sse41<_ISA_>;
        constexpr auto __avx2 = has_avx2<_ISA_>;

		if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (pd<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmp_pd_mask(as<__m128d>(__x), as<__m128d>(__y), _CMP_LE_OQ);
                else return as<_Tp_>(_mm_cmple_pd(as<__m128d>(__x), as<__m128d>(__y)));
            }
            else if constexpr (ps<_Type_>) {
                if constexpr (__avx512vl) return _mm_cmp_ps_mask(as<__m128>(__x), as<__m128>(__y), _CMP_LE_OQ);
                else return as<_Tp_>(_mm_cmple_ps(as<__m128>(__x), as<__m128>(__y)));
            }
            else if constexpr (__avx512vl) {
                if constexpr (epi64<_Type_>) return _mm_cmple_epi64_mask(as<__m128i>(__x), as<__m128i>(__y));
                else if constexpr (epu64<_Type_>) return _mm_cmple_epu64_mask(as<__m128i>(__x), as<__m128i>(__y));
                else if constexpr (epi32<_Type_>) return _mm_cmple_epi32_mask(as<__m128i>(__x), as<__m128i>(__y));
                else if constexpr (epu32<_Type_>) return _mm_cmple_epu32_mask(as<__m128i>(__x), as<__m128i>(__y));
                else if constexpr (__avx512bw) {
                    if constexpr (epi16<_Type_>) return _mm_cmple_epi16_mask(as<__m128i>(__x), as<__m128i>(__y));
                    else if constexpr (epu16<_Type_>) return _mm_cmple_epu16_mask(as<__m128i>(__x), as<__m128i>(__y));
                    else if constexpr (epi8<_Type_>) return _mm_cmple_epi8_mask(as<__m128i>(__x), as<__m128i>(__y));
                    else if constexpr (epu8<_Type_>) return _mm_cmple_epu8_mask(as<__m128i>(__x), as<__m128i>(__y));
                }
            }
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            if constexpr (pd<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmp_pd_mask(as<__m256d>(__x), as<__m256d>(__y), _CMP_LE_OQ);
                else return as<_Tp_>(_mm256_cmp_pd(as<__m256d>(__x), as<__m256d>(__y), _CMP_LE_OQ));
            }
            else if constexpr (ps<_Type_>) {
                if constexpr (__avx512vl) return _mm256_cmp_ps_mask(as<__m256>(__x), as<__m256>(__y), _CMP_LE_OQ);
                else return as<_Tp_>(_mm256_cmp_ps(as<__m256>(__x), as<__m256>(__y), _CMP_LE_OQ));
            }
            else if constexpr (__avx512vl) {
                if constexpr (epi64<_Type_>) return _mm256_cmple_epi64_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (epu64<_Type_>) return _mm256_cmple_epu64_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (epi32<_Type_>) return _mm256_cmple_epi32_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (epu32<_Type_>) return _mm256_cmple_epu32_mask(as<__m256i>(__x), as<__m256i>(__y));
                else if constexpr (__avx512bw) {
                    if constexpr (epi16<_Type_>) return _mm256_cmple_epi16_mask(as<__m256i>(__x), as<__m256i>(__y));
                    else if constexpr (epu16<_Type_>) return _mm256_cmple_epu16_mask(as<__m256i>(__x), as<__m256i>(__y));
                    else if constexpr (epi8<_Type_>) return _mm256_cmple_epi8_mask(as<__m256i>(__x), as<__m256i>(__y));
                    else if constexpr (epu8<_Type_>) return _mm256_cmple_epu8_mask(as<__m256i>(__x), as<__m256i>(__y));
                }
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (pd<_Type_>) return _mm512_cmple_pd_mask(as<__m512d>(__x), as<__m512d>(__y));
            else if constexpr (ps<_Type_>) return _mm512_cmple_ps_mask(as<__m512>(__x), as<__m512>(__y));
            else if constexpr (epi64<_Type_>) return _mm512_cmple_epi64_mask(as<__m512i>(__x), as<__m512i>(__y));
            else if constexpr (epu64<_Type_>) return _mm512_cmple_epu64_mask(as<__m512i>(__x), as<__m512i>(__y));
            else if constexpr (epi32<_Type_>) return _mm512_cmple_epi32_mask(as<__m512i>(__x), as<__m512i>(__y));
            else if constexpr (epu32<_Type_>) return _mm512_cmple_epu32_mask(as<__m512i>(__x), as<__m512i>(__y));
            else if constexpr (__avx512bw) {
                if constexpr (epi16<_Type_>) return _mm512_cmple_epi16_mask(as<__m512i>(__x), as<__m512i>(__y));
                else if constexpr (epu16<_Type_>) return _mm512_cmple_epu16_mask(as<__m512i>(__x), as<__m512i>(__y));
                else if constexpr (epi8<_Type_>) return _mm512_cmple_epi8_mask(as<__m512i>(__x), as<__m512i>(__y));
                else if constexpr (epu8<_Type_>) return _mm512_cmple_epu8_mask(as<__m512i>(__x), as<__m512i>(__y));
            }
        }
        
        if constexpr (arithmetic_type<_Tp_>) return __x <= __y;
        else return _Not<_ISA_, _Type_>()(_Less<_ISA_, _Type_>()(__y, __x));
	}
};

__RAZE_VX_NAMESPACE_END
