#pragma once 

#include <src/raze/vx/IntrinBitcast.h>
#include <src/raze/vx/bitwise/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Equal;

template <class _Type_>
struct _Equal<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline auto operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            const auto __equal_mask = _mm_cmpeq_epi32(
                __as<__m128i>(__left), __as<__m128i>(__right));

            const auto __rotated_mask = _mm_shuffle_epi32(__equal_mask, 0xB1);
            const auto __combined_mask = _mm_and_si128(__equal_mask, __rotated_mask);

            return __as<_IntrinType_>(__combined_mask);
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpeq_epi32(
                __as<__m128i>(__left), __as<__m128i>(__right)));
        }
        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpeq_epi16(
                __as<__m128i>(__left), __as<__m128i>(__right)));
        }
        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpeq_epi8(
                __as<__m128i>(__left), __as<__m128i>(__right)));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpeq_ps(
                __as<__m128>(__left), __as<__m128>(__right)));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpeq_pd(
                __as<__m128d>(__left), __as<__m128d>(__right)));
        }
	}
};

template <class _Type_> 
struct _Equal<arch::ISA::SSE41, 128, _Type_>:
    _Equal<arch::ISA::SSSE3, 128, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpeq_epi64(
                __as<__m128i>(__left), __as<__m128i>(__right)));

        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpeq_epi32(
                __as<__m128i>(__left), __as<__m128i>(__right)));

        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpeq_epi16(
                __as<__m128i>(__left), __as<__m128i>(__right)));

        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpeq_epi8(
                __as<__m128i>(__left), __as<__m128i>(__right)));

        else if constexpr (__is_ps_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpeq_ps(
                __as<__m128>(__left), __as<__m128>(__right)));

        else if constexpr (__is_pd_v<_Type_>)
            return __as<_IntrinType_>(_mm_cmpeq_pd(
                __as<__m128d>(__left), __as<__m128d>(__right)));
    }
};

template <class _Type_>
struct _Equal<arch::ISA::AVX, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmp_pd(
                __as<__m256d>(__left), __as<__m256d>(__right), _CMP_EQ_OQ));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return __as<_IntrinType_>(_mm256_cmp_ps(
                __as<__m256>(__left), __as<__m256>(__right), _CMP_EQ_OQ));
        }
        else {
            const auto __low = _Equal<arch::ISA::SSE42, 128, _Type_>()(
                __as<__m128i>(__left), __as<__m128i>(__right));

            const auto __high = _Equal<arch::ISA::SSE42, 128, _Type_>()(
                _mm256_extractf128_si256(__as<__m256i>(__left), 1),
                _mm256_extractf128_si256(__as<__m256i>(__right), 1));

            return __as<_IntrinType_>(_mm256_insertf128_si256(
                __as<__m256i>(__low), __high, 1));
        }
    }
};

template <class _Type_> struct _Equal<arch::ISA::FMA3, 256, _Type_> : _Equal<arch::ISA::AVX, 256, _Type_> {};

template <class _Type_>
struct _Equal<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline auto operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
        if constexpr (__is_pd_v<_Type_>)
            return __as<_IntrinType_>(_mm256_cmp_pd(
                __as<__m256d>(__left), __as<__m256d>(__right), _CMP_EQ_OQ));

        else if constexpr (__is_ps_v<_Type_>)
            return __as<_IntrinType_>(_mm256_cmp_ps(
                __as<__m256>(__left), __as<__m256>(__right), _CMP_EQ_OQ));

        else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
            return __as<_IntrinType_>(_mm256_cmpeq_epi64(
                __as<__m256i>(__left), __as<__m256i>(__right)));

        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
            return __as<_IntrinType_>(_mm256_cmpeq_epi32(
                __as<__m256i>(__left), __as<__m256i>(__right)));

        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
            return __as<_IntrinType_>(_mm256_cmpeq_epi16(
                __as<__m256i>(__left), __as<__m256i>(__right)));

        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
            return __as<_IntrinType_>(_mm256_cmpeq_epi8(
                __as<__m256i>(__left), __as<__m256i>(__right)));
	}
};

template <class _Type_> struct _Equal<arch::ISA::AVX2FMA3, 256, _Type_> : _Equal<arch::ISA::AVX2, 256, _Type_> {};

template <class _Type_>
struct _Equal<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_static_operator raze_always_inline auto operator()(
		_IntrinType_ __left,
		_IntrinType_ __right) raze_const_operator noexcept
	{
        if constexpr (__is_pd_v<_Type_>) {
            return _mm512_cmpeq_pd_mask(__as<__m512d>(__left), __as<__m512d>(__right));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            return _mm512_cmpeq_ps_mask(__as<__m512>(__left), __as<__m512>(__right));
        }
        else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            return _mm512_cmpeq_epi64_mask(__as<__m512i>(__left), __as<__m512i>(__right));
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
            return _mm512_cmpeq_epi32_mask(__as<__m512i>(__left), __as<__m512i>(__right));
        }
        else {
            const auto __compared_low128 = _Equal<arch::ISA::SSE42, 128, _Type_>()(
                __as<__m128i>(__left), __as<__m128i>(__right));

            const auto __compared2_low128 = _Equal<arch::ISA::SSE42, 128, _Type_>()(
                _mm256_extractf128_si256(__as<__m256i>(__left), 1), _mm256_extractf128_si256(__as<__m256i>(__right), 1));

            const auto __compared_high128 = _Equal<arch::ISA::SSE42, 128, _Type_>()(
                _mm512_extracti32x4_epi32(__as<__m512i>(__left), 2), _mm512_extracti32x4_epi32(__as<__m512i>(__right), 2));

            const auto __compared2_high128 = _Equal<arch::ISA::SSE42, 128, _Type_>()(
                _mm512_extracti32x4_epi32(__as<__m512i>(__left), 3), _mm512_extracti32x4_epi32(__as<__m512i>(__right), 3));

            auto __result = __as<__m512i>(__compared_low128);

            __result = _mm512_inserti32x4(__result, __compared2_low128, 1);
            __result = _mm512_inserti32x4(__result, __compared_high128, 2);

            return __as<_IntrinType_>(_mm512_inserti32x4(__result, __compared2_high128, 3));
        }
	}
};

template <class _Type_> 
struct _Equal<arch::ISA::AVX512BW, 512, _Type_>:
    _Equal<arch::ISA::AVX512F, 512, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_Type_>)
            return _mm512_cmpeq_pd_mask(__as<__m512d>(__left), __as<__m512d>(__right));

        else if constexpr (__is_ps_v<_Type_>)
            return _mm512_cmpeq_ps_mask(__as<__m512>(__left), __as<__m512>(__right));

        else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
            return _mm512_cmpeq_epi64_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
            return _mm512_cmpeq_epi32_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
            return _mm512_cmpeq_epi16_mask(__as<__m512i>(__left), __as<__m512i>(__right));

        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
            return _mm512_cmpeq_epi8_mask(__as<__m512i>(__left), __as<__m512i>(__right));
    }
};

template <class _Type_>
struct _Equal<arch::ISA::AVX512VLF, 256, _Type_>:
    _Equal<arch::ISA::AVX2, 256, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_Type_>)
            return _mm256_cmp_pd_mask(__as<__m256d>(__left), __as<__m256d>(__right), _CMP_EQ_OQ);

        else if constexpr (__is_ps_v<_Type_>)
            return _mm256_cmp_ps_mask(__as<__m256>(__left), __as<__m256>(__right), _CMP_EQ_OQ);

        else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
            return _mm256_cmpeq_epi64_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
            return _mm256_cmpeq_epi32_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else
            return _Equal<arch::ISA::AVX2, 256, _Type_>()(__left, __right);
    }
};

template <class _Type_> 
struct _Equal<arch::ISA::AVX512VLBW, 256, _Type_>: 
    _Equal<arch::ISA::AVX512VLF, 256, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi16_v<_Type_>)
            return _mm256_cmpeq_epi16_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm256_cmpeq_epu16_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epi8_v<_Type_>)
            return _mm256_cmpeq_epi8_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm256_cmpeq_epu8_mask(__as<__m256i>(__left), __as<__m256i>(__right));

        else
            return _Equal<arch::ISA::AVX512VLF, 256, _Type_>()(__left, __right);
    }
};

template <class _Type_>
struct _Equal<arch::ISA::AVX512VLF, 128, _Type_>:
    _Equal<arch::ISA::AVX2, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_Type_>)
            return _mm_cmp_pd_mask(__as<__m128d>(__left), __as<__m128d>(__right), _CMP_EQ_OQ);

        else if constexpr (__is_ps_v<_Type_>)
            return _mm_cmp_ps_mask(__as<__m128>(__left), __as<__m128>(__right), _CMP_EQ_OQ);

        else if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
            return _mm_cmpeq_epi64_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
            return _mm_cmpeq_epi32_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else
            return _Equal<arch::ISA::SSE42, 128, _Type_>()(__left, __right);
    }
};

template <class _Type_> 
struct _Equal<arch::ISA::AVX512VLBW, 128, _Type_>: 
    _Equal<arch::ISA::AVX512VLF, 128, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi16_v<_Type_>)
            return _mm_cmpeq_epi16_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epu16_v<_Type_>)
            return _mm_cmpeq_epu16_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epi8_v<_Type_>)
            return _mm_cmpeq_epi8_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else if constexpr (__is_epu8_v<_Type_>)
            return _mm_cmpeq_epu8_mask(__as<__m128i>(__left), __as<__m128i>(__right));

        else
            return _Equal<arch::ISA::AVX512VLF, 128, _Type_>()(__left, __right);
    }
};


template <class _Type_> struct _Equal<arch::ISA::SSE3, 128, _Type_> : _Equal<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::SSSE3, 128, _Type_> : _Equal<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::SSE42, 128, _Type_> : _Equal<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX, 128, _Type_> : _Equal<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::FMA3, 128, _Type_> : _Equal<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX2, 128, _Type_> : _Equal<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX2FMA3, 128, _Type_> : _Equal<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Equal<arch::ISA::AVX512DQ, 512, _Type_> : _Equal<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512BWDQ, 512, _Type_> : _Equal<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI, 512, _Type_> : _Equal<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI2, 512, _Type_> : _Equal<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Equal<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Equal<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Equal<arch::ISA::AVX512VLDQ, 256, _Type_> : _Equal<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Equal<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Equal<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Equal<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Equal<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Equal<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Equal<arch::ISA::AVX512VLDQ, 128, _Type_> : _Equal<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Equal<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Equal<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Equal<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Equal<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Equal<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Equal<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END

