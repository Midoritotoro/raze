#pragma once 

#include <raze/memory/PointerToIntegral.h>
#include <raze/math/IntegralTypesConversions.h>

#include <src/raze/vx/hw/x86/mask/MaskTypeSelector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _To_mask {
	template <intrin_type _Tp_>
	raze_nodiscard raze_always_inline __mmask_for_elements_t<sizeof(_Tp_) / sizeof(_Type_)> operator()(_Tp_ __x) const noexcept {
        constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
        constexpr auto __avx512dq = __has_avx512dq_support_v<_ISA_>;
        constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
        constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;

		if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512vl && __avx512dq) return _mm_movepi64_mask(__as<__m128i>(__x));
                else return _mm_movemask_pd(__as<__m128d>(__x));
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512vl && __avx512dq) return _mm_movepi32_mask(__as<__m128i>(__x));
                else return _mm_movemask_ps(__as<__m128>(__x));
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__avx512vl && __avx512bw) return _mm_movepi16_mask(__as<__m128i>(__x));
                else return _mm_movemask_epi8(_mm_packs_epi16(__as<__m128i>(__x), _mm_setzero_si128()));
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__avx512vl && __avx512bw) return _mm_movepi16_mask(__as<__m128i>(__x));
                return _mm_movemask_epi8(__as<__m128i>(__x));
            }
		}
	}
};


__RAZE_VX_NAMESPACE_END
