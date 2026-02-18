#pragma once 

#include <src/raze/datapar/bitwise/ToMask.h>
#include <src/raze/datapar/compare/Greater.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_DesiredType_>
struct _Simd_less_equal;

template <class _DesiredType_>
struct _Simd_less_equal<arch::ISA::SSE2, 128, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_DesiredType_>)
            return __intrin_bitcast<_IntrinType_>(_mm_cmple_pd(__intrin_bitcast<__m128d>(__left), __intrin_bitcast<__m128d>(__right)));

        else if constexpr (__is_ps_v<_DesiredType_>)
            return __intrin_bitcast<_IntrinType_>(_mm_cmple_ps(__intrin_bitcast<__m128>(__left), __intrin_bitcast<__m128>(__right)));

        else
            return _Simd_bit_not<arch::ISA::SSE2, 128>()(_Simd_greater<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right));
    }
};

template <class _DesiredType_>
struct _Simd_less_equal<arch::ISA::AVX2, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_bit_not<arch::ISA::AVX2, 256>()(_Simd_greater<arch::ISA::AVX2, 256, _DesiredType_>()(__left, __right));
    }
};

template <class _DesiredType_>
struct _Simd_less_equal<arch::ISA::AVX512F, 512, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return _mm512_cmple_epi64_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            return _mm512_cmple_epu64_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epi32_v<_DesiredType_>) {
            return _mm512_cmple_epi32_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_epu32_v<_DesiredType_>) {
            return _mm512_cmple_epu32_mask(__intrin_bitcast<__m512i>(__left), __intrin_bitcast<__m512i>(__right));
        }
        else if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm512_cmple_pd_mask(__intrin_bitcast<__m512d>(__left), __intrin_bitcast<__m512d>(__right));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm512_cmple_ps_mask(__intrin_bitcast<__m512>(__left), __intrin_bitcast<__m512>(__right));
        }
        else {
            const auto __compared_low128 = _Simd_less_equal<arch::ISA::SSE42, 128, _DesiredType_>()(
                __intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right));

            const auto __compared2_low128 = _Simd_less_equal<arch::ISA::SSE42, 128, _DesiredType_>()(
                _mm256_extractf128_si256(__intrin_bitcast<__m256i>(__left), 1), _mm256_extractf128_si256(__intrin_bitcast<__m256i>(__right), 1));

            const auto __compared_high128 = _Simd_less_equal<arch::ISA::SSE42, 128, _DesiredType_>()(
                _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__left), 2), _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__right), 2));

            const auto __compared2_high128 = _Simd_less_equal<arch::ISA::SSE42, 128, _DesiredType_>()(
                _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__left), 3), _mm512_extracti32x4_epi32(__intrin_bitcast<__m512i>(__right), 3));

            auto __result = __intrin_bitcast<__m512i>(__compared_low128);

            __result = _mm512_inserti32x4(__result, __compared2_low128, 1);
            __result = _mm512_inserti32x4(__result, __compared_high128, 2);

            return _Simd_to_mask<arch::ISA::AVX512F, 512, _DesiredType_>()(_mm512_inserti32x4(__result, __compared2_high128, 3));
        }
    }
};

template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512BW, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_less_equal<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_less_equal<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
