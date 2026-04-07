#pragma once 

#include <src/raze/datapar/reduce/ReduceAddType.h>
#include <src/raze/datapar/arithmetic/Add.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _Reduce_add;

template <class _DesiredType_>
struct _Reduce_add<arch::ISA::SSE2, 128, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
        __reduce_type<_DesiredType_> operator()(_IntrinType_ __vector) raze_const_operator noexcept
    {
        using _ReduceType = __reduce_type<_DesiredType_>;

        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
#if defined(raze_processor_x86_32)
            return static_cast<_ReduceType>(_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__vector)) +
                _Extract<arch::ISA::SSE2, 128, int32>()(__vector, 2));
#else 
            return static_cast<_ReduceType>(_mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__vector)) +
                _Extract<arch::ISA::SSE2, 128, int64>()(__vector, 1));
#endif // defined(raze_processor_x86_32)
        }
        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            const auto __shuffled1 = _mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), _MM_SHUFFLE(2, 3, 0, 1));
            const auto __reduce1 = _mm_add_epi32(__intrin_bitcast<__m128i>(__vector), __shuffled1);

            const auto __shuffled2 = _mm_shuffle_epi32(__reduce1, _MM_SHUFFLE(1, 0, 3, 2));
            const auto __reduce2 = _mm_add_epi32(__intrin_bitcast<__m128i>(__vector), __shuffled2);

            return _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__vector));
        }
        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
            const auto __zeros = _mm_setzero_si128();

            const auto __low = _mm_unpacklo_epi16(__intrin_bitcast<__m128i>(__vector), __zeros);
            const auto __high = _mm_unpackhi_epi16(__intrin_bitcast<__m128i>(__vector), __zeros);

            const auto __reduce1 = _mm_add_epi32(__low, __high);

            const auto __shuffle1 = _mm_shuffle_epi32(__reduce1, _MM_SHUFFLE(2, 3, 0, 1));
            const auto __reduce2 = _mm_add_epi32(__reduce1, __shuffle1);

            const auto __shuffled2 = _mm_shuffle_epi32(__reduce2, _MM_SHUFFLE(1, 0, 3, 2));
            const auto __reduce3 = _mm_add_epi32(__reduce2, __shuffled2);

            return _mm_cvtsi128_si32(__reduce3);
        }
        else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
            const auto __first_reduce = _mm_sad_epu8(__intrin_bitcast<__m128i>(__vector), _mm_setzero_si128());
#if defined(raze_processor_x86_32)
            return static_cast<_ReduceType>(_mm_cvtsi128_si32(__first_reduce)
                + _Extract<arch::ISA::SSE2, 128, int32>()(__first_reduce, 2));
#else
            return static_cast<_ReduceType>(_mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__first_reduce))
                + _Extract<arch::ISA::SSE2, 128, int64>()(__first_reduce, 1));
#endif // defined(raze_processor_x86_32)
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            const auto __shuffled1 = _mm_movehl_ps(__intrin_bitcast<__m128>(__vector), __intrin_bitcast<__m128>(__vector));
            const auto __reduce1 = _mm_add_ps(__intrin_bitcast<__m128>(__vector), __shuffled1);

            const auto __shuffled2 = _mm_shuffle_ps(__reduce1, __reduce1, 0x1);
            const auto __reduce2 = _mm_add_ss(__reduce1, __shuffled2);

            return _mm_cvtss_f32(__reduce2);
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            const auto __reduce = _mm_add_sd(__intrin_bitcast<__m128d>(__vector), _mm_unpackhi_pd(
                __intrin_bitcast<__m128d>(__vector), __intrin_bitcast<__m128d>(__vector)));
            return _mm_cvtsd_f64(__reduce);
        }
	}
};

template <class _DesiredType_> 
struct _Reduce_add<arch::ISA::SSSE3, 128, _DesiredType_>:
    _Reduce_add<arch::ISA::SSE3, 128, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline
        __reduce_type<_DesiredType_> operator()(_IntrinType_ __vector) raze_const_operator noexcept
    {
        using _ReduceType = __reduce_type<_DesiredType_>;

        if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            const auto __reduce4 = _mm_hadd_epi32(__intrin_bitcast<__m128i>(__vector), __intrin_bitcast<__m128i>(__vector));
            return static_cast<_ReduceType>(_mm_cvtsi128_si32(_mm_hadd_epi32(__reduce4, __reduce4)));
        }
        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
            const auto __zeros = _mm_setzero_si128();

            const auto __reduce2 = _mm_hadd_epi16(__intrin_bitcast<__m128i>(__vector), __zeros);
            const auto __reduce3 = _mm_unpacklo_epi16(__reduce2, __zeros);

            const auto __reduce4 = _mm_hadd_epi32(__reduce3, __zeros);
            const auto __reduce5 = _mm_hadd_epi32(__reduce4, __zeros);

            return static_cast<_ReduceType>(_mm_cvtsi128_si32(__reduce5));
        }
        else {
            return _Reduce_add<arch::ISA::SSE2, 128, _DesiredType_>()(__vector);
        }
    }
};

template <class _DesiredType_> struct _Reduce_add<arch::ISA::SSE3, 128, _DesiredType_> : _Reduce_add<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::SSE41, 128, _DesiredType_> : _Reduce_add<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::SSE42, 128, _DesiredType_> : _Reduce_add<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX, 128, _DesiredType_> : _Reduce_add<arch::ISA::SSE42, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::FMA3, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX2, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX2FMA3, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX2, 128, _DesiredType_> {};

template <class _DesiredType_> 
struct _Reduce_add<arch::ISA::AVX, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline __reduce_type<_DesiredType_> 
        operator()(_IntrinType_ __vector) raze_const_operator noexcept
    {
        const auto __low = __intrin_bitcast<__m128>(__vector);
        const auto __high = _mm256_extractf128_ps(__intrin_bitcast<__m256>(__vector), 1);

        const auto __vertical_sum = _Add<arch::ISA::AVX, 128, _DesiredType_>()(__low, __high);
        return _Reduce_add<arch::ISA::AVX, 128, _DesiredType_>()(__vertical_sum);
    }
};

template <class _DesiredType_>
struct _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>:
    _Reduce_add<arch::ISA::AVX, 256, _DesiredType_> 
{
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline 
        __reduce_type<_DesiredType_> operator()(_IntrinType_ __vector) raze_const_operator noexcept
    {
        using _ReduceType = __reduce_type<_DesiredType_>;

        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
            const auto __low64   = __intrin_bitcast<__m128i>(__vector);
            const auto __high64  = _mm256_extracti128_si256(__intrin_bitcast<__m256i>(__vector), 1);

            const auto __reduce  = _mm_add_epi64(__low64, __high64);
            return _Reduce_add<arch::ISA::SSSE3, 128, _DesiredType_>()(__reduce);
        }
        if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            const auto __zeros = _mm256_setzero_si256();

            const auto __reduce4 = _mm256_hadd_epi32(__intrin_bitcast<__m256i>(__vector), __zeros);
            const auto __reduce5 = _mm256_permute4x64_epi64(__reduce4, 0xD8);

            const auto __reduce6 = _mm256_hadd_epi32(__reduce5, __zeros);
            const auto __reduce7 = _mm256_hadd_epi32(__reduce6, __zeros);

            return static_cast<_ReduceType>(_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__reduce7)));
        }
        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
            const auto __zeros = _mm256_setzero_si256();

            const auto __reduce2 = _mm256_hadd_epi16(__intrin_bitcast<__m256i>(__vector), __zeros);
            const auto __reduce3 = _mm256_unpacklo_epi16(__reduce2, __zeros);

            const auto __reduce4 = _mm256_hadd_epi32(__reduce3, __zeros);
            const auto __reduce5 = _mm256_permute4x64_epi64(__reduce4, 0xD8);

            const auto __reduce6 = _mm256_hadd_epi32(__reduce5, __zeros);
            const auto __reduce7 = _mm256_hadd_epi32(__reduce6, __zeros);

            return static_cast<_ReduceType>(_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__reduce7)));
        }
        else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
            const auto __reduce1 = _mm256_sad_epu8(__intrin_bitcast<__m256i>(__vector), _mm256_setzero_si256());

            const auto __low64  = _mm256_castsi256_si128(__reduce1);
            const auto __high64 = _mm256_extracti128_si256(__reduce1, 1);

            const auto __reduce8 = _mm_add_epi64(__low64, __high64);
            return _Reduce_add<arch::ISA::SSSE3, 128, int64>()(__reduce8);
        }
        else {
            return _Reduce_add<arch::ISA::AVX, 256, _DesiredType_>()(__vector);
        }
	}
};

template <class _DesiredType_>
struct _Reduce_add<arch::ISA::AVX512F, 512, _DesiredType_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline
        __reduce_type<_DesiredType_> operator()(_IntrinType_ __vector) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
            return _mm512_reduce_add_epi64(__intrin_bitcast<__m512i>(__vector));
        }
        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            return _mm512_reduce_add_epi32(__intrin_bitcast<__m512i>(__vector));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm512_reduce_add_ps(__intrin_bitcast<__m512>(__vector));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm512_reduce_add_pd(__intrin_bitcast<__m512d>(__vector));
        }
        else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>) {
            return _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>()(__intrin_bitcast<__m256i>(__vector)) +
                _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>()(_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1));
        }
        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
            return _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>()(__intrin_bitcast<__m256i>(__vector)) +
                _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>()(_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1));
        }
	}
};

template <class _DesiredType_>
struct _Reduce_add<arch::ISA::AVX512BW, 512, _DesiredType_>: 
    _Reduce_add<arch::ISA::AVX512F, 512, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline
        __reduce_type<_DesiredType_> operator()(_IntrinType_ __vector) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>)
            return _mm512_reduce_add_epi64(__intrin_bitcast<__m512i>(__vector));

        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>)
            return _mm512_reduce_add_epi32(__intrin_bitcast<__m512i>(__vector));

        else if constexpr (__is_ps_v<_DesiredType_>)
            return _mm512_reduce_add_ps(__intrin_bitcast<__m512>(__vector));

        else if constexpr (__is_pd_v<_DesiredType_>)
            return _mm512_reduce_add_pd(__intrin_bitcast<__m512d>(__vector));

        else if constexpr (__is_epi8_v<_DesiredType_> || __is_epu8_v<_DesiredType_>)
            return _mm512_reduce_add_epi64(_mm512_sad_epu8(__intrin_bitcast<__m512i>(__vector), _mm512_setzero_si512()));

        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>)
            return _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>()(__intrin_bitcast<__m256i>(__vector)) +
                _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_>()(_mm512_extractf64x4_pd(__intrin_bitcast<__m512d>(__vector), 1));
    }
};

template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Reduce_add<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Reduce_add<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Reduce_add<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Reduce_add<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Reduce_add<arch::ISA::FMA3, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX2FMA3, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Reduce_add<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Reduce_add<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
