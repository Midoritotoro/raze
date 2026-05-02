#pragma once 

#include <raze/memory/PointerToIntegral.h>
#include <raze/math/IntegralTypesConversions.h>
#include <src/raze/vx/storage/BestMaskChunk.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _To_mask {
	template <intrin_type _Tp_>
	raze_nodiscard raze_always_inline typename __mask_type<sizeof(_Tp_) / sizeof(_Type_)>::type operator()(_Tp_ __x) const noexcept {
        using _MaskType = typename __mask_type<sizeof(_Tp_) / sizeof(_Type_)>::type;

        constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
        constexpr auto __avx512dq = __has_avx512dq_support_v<_ISA_>;
        constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
        constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
        constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;

		if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return _mm_movepi64_mask(__as<__m128i>(__x));
                    else return _mm_cmplt_epi64_mask(__as<__m128i>(__x), _mm_setzero_si128());
                }
                else return _mm_movemask_pd(__as<__m128d>(__x));
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return _mm_movepi32_mask(__as<__m128i>(__x));
                    else return _mm_cmplt_epi32_mask(__as<__m128i>(__x), _mm_setzero_si128());
                }
                else return _mm_movemask_ps(__as<__m128>(__x));
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__avx512vl && __avx512bw) return _mm_movepi16_mask(__as<__m128i>(__x));
                else return _mm_movemask_epi8(_mm_packs_epi16(__as<__m128i>(__x), _mm_setzero_si128()));
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__avx512vl && __avx512bw) return _mm_movepi8_mask(__as<__m128i>(__x));
                return _mm_movemask_epi8(__as<__m128i>(__x));
            }
		}
        else if constexpr (sizeof(_Tp_) == 32) {
            auto __fallback_avx = [=] () raze_always_inline_lambda {
                constexpr auto __half_bits = (sizeof(_Tp_) / sizeof(_Type_)) >> 1;

                const auto __low = _To_mask<arch::ISA::SSE42, _Type_>()(__as<__m256i>(__x));
                const auto __high = _To_mask<arch::ISA::SSE42, _Type_>()(_mm256_extractf128_si256(__as<__m256i>(__x), 1));

                return ((static_cast<_MaskType>(__high) << __half_bits) | static_cast<_MaskType>(__low));
            };

            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return _mm256_movepi64_mask(__as<__m256i>(__x));
                    else return _mm256_cmplt_epi64_mask(__as<__m256i>(__x), _mm256_setzero_si256());
                }
                else return _mm256_movemask_pd(__as<__m256d>(__x));
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512vl) {
                    if constexpr (__avx512dq) return _mm256_movepi32_mask(__as<__m256i>(__x));
                    else return _mm256_cmplt_epi32_mask(__as<__m256i>(__x), _mm256_setzero_si256());
                }
                else return _mm256_movemask_ps(__as<__m256>(__x));
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__avx512vl && __avx512bw) return _mm256_movepi16_mask(__as<__m256i>(__x));
                else if constexpr (__avx2) return _mm256_movemask_epi8(_mm256_permute4x64_epi64(_mm256_packs_epi16(__as<__m256i>(__x), _mm256_setzero_si256()), 0xD8));
                else return __fallback_avx();
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__avx512vl && __avx512bw) return _mm256_movepi8_mask(__as<__m256i>(__x));
                else if constexpr (__avx2) return _mm256_movemask_epi8(__as<__m256i>(__x));
                else return __fallback_avx();
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            auto __fallback_avx512f = []() raze_always_inline_lambda{
                constexpr auto __half_bits = (sizeof(_Tp_) / sizeof(_Type_)) >> 1;

                const auto __low = _To_mask<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x));
                const auto __high = _To_mask<arch::ISA::AVX2, _Type_>()(_mm512_extractf64x4_pd(__as<__m512d>(__x), 1));

                return ((static_cast<_MaskType>(__high) << __half_bits) | static_cast<_MaskType>(__low));
            };

            if constexpr (sizeof(_Type_) == 8) {
                if constexpr (__avx512dq) return _mm512_movepi64_mask(__as<__m512i>(__x));
                else return _mm512_cmplt_epi64_mask(__as<__m512i>(__x), _mm512_setzero_si512());
            }
            else if constexpr (sizeof(_Type_) == 4) {
                if constexpr (__avx512dq) return _mm512_movepi32_mask(__as<__m512i>(__x));
                else return _mm512_cmplt_epi32_mask(__as<__m512i>(__x), _mm512_setzero_si512());
            }
            else if constexpr (sizeof(_Type_) == 2) {
                if constexpr (__avx512bw) return _mm512_movepi16_mask(__as<__m512i>(__x));
                else return __fallback_avx512f();
            }
            else if constexpr (sizeof(_Type_) == 1) {
                if constexpr (__avx512bw) return _mm512_movepi8_mask(__as<__m512i>(__x));
                else return __fallback_avx512f();
            }
        }
	}
};


__RAZE_VX_NAMESPACE_END
