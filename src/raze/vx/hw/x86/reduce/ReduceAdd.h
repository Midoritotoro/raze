 #pragma once 

#include <src/raze/vx/hw/x86/reduce/ReduceAddType.h>
#include <src/raze/vx/hw/x86/arithmetic/Add.h>
#include <src/raze/vx/hw/x86/access/Extract.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Reduce_add {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline __reduce_type<_Type_> operator()(_Tp_ __x) const noexcept {
        using _ReduceType = __reduce_type<_Type_>;

        if constexpr (sizeof(_Tp_) == 16) {
            if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
#if defined(raze_processor_x86_32)
                return static_cast<_ReduceType>(_mm_cvtsi128_si32(__as<__m128i>(__x)) +
                    _Extract<arch::ISA::SSE2, i32>()(__x, 2));
#else 
                return static_cast<_ReduceType>(_mm_cvtsi128_si64(__as<__m128i>(__x)) +
                    _Extract<arch::ISA::SSE2, i64>()(__x, 1));
#endif // defined(raze_processor_x86_32)
            }
            else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
                if constexpr (__has_ssse3_support_v<_ISA_>) {
                    const auto __reduce4 = _mm_hadd_epi32(__as<__m128i>(__x), __as<__m128i>(__x));
                    return static_cast<_ReduceType>(_mm_cvtsi128_si32(_mm_hadd_epi32(__reduce4, __reduce4)));
                }
                else {
                    const auto __shuffled1 = _mm_shuffle_epi32(__as<__m128i>(__x), _MM_SHUFFLE(2, 3, 0, 1));
                    const auto __reduce1 = _mm_add_epi32(__as<__m128i>(__x), __shuffled1);

                    const auto __shuffled2 = _mm_shuffle_epi32(__reduce1, _MM_SHUFFLE(1, 0, 3, 2));
                    const auto __reduce2 = _mm_add_epi32(__reduce1, __shuffled2);

                    return _mm_cvtsi128_si32(__as<__m128i>(__reduce2));
                }
            }
            else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
                if constexpr (__has_ssse3_support_v<_ISA_>) {
                    const auto __zeros = _mm_setzero_si128();

                    const auto __reduce2 = _mm_hadd_epi16(__as<__m128i>(__x), __zeros);
                    const auto __reduce3 = _mm_unpacklo_epi16(__reduce2, __zeros);

                    const auto __reduce4 = _mm_hadd_epi32(__reduce3, __zeros);
                    const auto __reduce5 = _mm_hadd_epi32(__reduce4, __zeros);

                    return static_cast<_ReduceType>(_mm_cvtsi128_si32(__reduce5));
                }
                else {
                    const auto __zeros = _mm_setzero_si128();

                    const auto __low = _mm_unpacklo_epi16(__as<__m128i>(__x), __zeros);
                    const auto __high = _mm_unpackhi_epi16(__as<__m128i>(__x), __zeros);

                    const auto __reduce1 = _mm_add_epi32(__low, __high);

                    const auto __shuffle1 = _mm_shuffle_epi32(__reduce1, _MM_SHUFFLE(2, 3, 0, 1));
                    const auto __reduce2 = _mm_add_epi32(__reduce1, __shuffle1);

                    const auto __shuffled2 = _mm_shuffle_epi32(__reduce2, _MM_SHUFFLE(1, 0, 3, 2));
                    return _mm_cvtsi128_si32(_mm_add_epi32(__reduce2, __shuffled2));
                }
            }
            else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
                const auto __first_reduce = _mm_sad_epu8(__as<__m128i>(__x), _mm_setzero_si128());
#if defined(raze_processor_x86_32)
                return static_cast<_ReduceType>(_mm_cvtsi128_si32(__first_reduce)
                    + _Extract<arch::ISA::SSE2, i32>()(__first_reduce, 2));
#else
                return static_cast<_ReduceType>(_mm_cvtsi128_si64(__as<__m128i>(__first_reduce))
                    + _Extract<arch::ISA::SSE2, i64>()(__first_reduce, 1));
#endif // defined(raze_processor_x86_32)
            }
            else if constexpr (__is_ps_v<_Type_>) {
                if constexpr (__has_sse3_support_v<_ISA_>) {
                    const auto __reduce4 = _mm_hadd_ps(__as<__m128>(__x), __as<__m128>(__x));
                    return static_cast<_ReduceType>(_mm_cvtss_f32(_mm_hadd_ps(__reduce4, __reduce4)));
                }
                else {
                    const auto __shuffled1 = _mm_movehl_ps(__as<__m128>(__x), __as<__m128>(__x));
                    const auto __reduce1 = _mm_add_ps(__as<__m128>(__x), __shuffled1);

                    const auto __shuffled2 = _mm_shuffle_ps(__reduce1, __reduce1, 0x1);
                    return _mm_cvtss_f32(_mm_add_ss(__reduce1, __shuffled2));
                }
            }
            else if constexpr (__is_pd_v<_Type_>) {
                if constexpr (__has_sse3_support_v<_ISA_>) return _mm_cvtsd_f64(_mm_hadd_pd(__as<__m128d>(__x), __as<__m128d>(__x)));
                else return _mm_cvtsd_f64(_mm_add_sd(__as<__m128d>(__x), _mm_unpackhi_pd(__as<__m128d>(__x), __as<__m128d>(__x))));
            }
        }
        else if constexpr (sizeof(_Tp_) == 32) {
            if constexpr (__has_avx2_support_v<_ISA_>) {
                if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
                    const auto __low64 = __as<__m128i>(__x);
                    const auto __high64 = _mm256_extracti128_si256(__as<__m256i>(__x), 1);

                    const auto __reduce = _mm_add_epi64(__low64, __high64);
                    return _Reduce_add<arch::ISA::SSSE3, _Type_>()(__reduce);
                }
                if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
                    const auto __zeros = _mm256_setzero_si256();

                    const auto __reduce4 = _mm256_hadd_epi32(__as<__m256i>(__x), __zeros);
                    const auto __reduce5 = _mm256_permute4x64_epi64(__reduce4, 0xD8);

                    const auto __reduce6 = _mm256_hadd_epi32(__reduce5, __zeros);
                    const auto __reduce7 = _mm256_hadd_epi32(__reduce6, __zeros);

                    return static_cast<_ReduceType>(_mm_cvtsi128_si32(__as<__m128i>(__reduce7)));
                }
                else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
                    const auto __zeros = _mm256_setzero_si256();

                    const auto __reduce2 = _mm256_hadd_epi16(__as<__m256i>(__x), __zeros);
                    const auto __reduce3 = _mm256_unpacklo_epi16(__reduce2, __zeros);

                    const auto __reduce4 = _mm256_hadd_epi32(__reduce3, __zeros);
                    const auto __reduce5 = _mm256_permute4x64_epi64(__reduce4, 0xD8);

                    const auto __reduce6 = _mm256_hadd_epi32(__reduce5, __zeros);
                    const auto __reduce7 = _mm256_hadd_epi32(__reduce6, __zeros);

                    return static_cast<_ReduceType>(_mm_cvtsi128_si32(__as<__m128i>(__reduce7)));
                }
                else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
                    const auto __reduce1 = _mm256_sad_epu8(__as<__m256i>(__x), _mm256_setzero_si256());

                    const auto __low64 = _mm256_castsi256_si128(__reduce1);
                    const auto __high64 = _mm256_extracti128_si256(__reduce1, 1);

                    const auto __reduce8 = _mm_add_epi64(__low64, __high64);
                    return _Reduce_add<arch::ISA::SSSE3, i64>()(__reduce8);
                }
            }
            else {
                const auto __low = __as<__m128>(__x);
                const auto __high = _mm256_extractf128_ps(__as<__m256>(__x), 1);

                const auto __vertical_sum = _Add<arch::ISA::AVX, _Type_>()(__low, __high);
                return _Reduce_add<arch::ISA::AVX, _Type_>()(__vertical_sum);
            }
        }
        else if constexpr (sizeof(_Tp_) == 64) {
            if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm512_reduce_add_epi64(__as<__m512i>(__x));
            else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm512_reduce_add_epi32(__as<__m512i>(__x));
            else if constexpr (__is_ps_v<_Type_>) return _mm512_reduce_add_ps(__as<__m512>(__x));
            else if constexpr (__is_pd_v<_Type_>)  return _mm512_reduce_add_pd(__as<__m512d>(__x));
            else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
                return _Reduce_add<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x)) + _Reduce_add<arch::ISA::AVX2, _Type_>()(_mm512_extractf64x4_pd(__as<__m512d>(__x), 1));
            else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
                return _Reduce_add<arch::ISA::AVX2, _Type_>()(__as<__m256i>(__x)) + _Reduce_add<arch::ISA::AVX2, _Type_>()(_mm512_extractf64x4_pd(__as<__m512d>(__x), 1));
        }
        else {
            return __x;
        }
	}

    template <intrin_or_arithmetic_type	_Tp_, raw_mask_type	_Mask_>
    raze_nodiscard raze_always_inline __reduce_type<_Type_> operator()(_Tp_ __x, _Mask_ __mask) const noexcept {
        return _Selectz<_ISA_, _Type_>()(__x, __mask);
    }

    template <intrin_or_arithmetic_type	_Tp_, raw_mask_type	_Mask_>
    raze_nodiscard raze_always_inline __reduce_type<_Type_> operator()(_Tp_ __x, _Mask_ __mask, _Tp_ __src) const noexcept {
        return _Select<_ISA_, _Type_>()(__x, __src, __mask);
    }
};

__RAZE_VX_NAMESPACE_END
