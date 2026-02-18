#pragma once 

#include <src/raze/datapar/compare/Less.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_DesiredType_>
struct _Simd_greater;

template <class _DesiredType_>
struct _Simd_greater<arch::ISA::SSE2, 128, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::SSE2, 128, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_>
struct _Simd_greater<arch::ISA::SSE42, 128, _DesiredType_>:
    _Simd_greater<arch::ISA::SSE41, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_>) {
            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi64(
                __intrin_bitcast<__m128i>(__left), __intrin_bitcast<__m128i>(__right)));
        }
        else if constexpr (__is_epu64_v<_DesiredType_>) {
            const auto __sign_64bit = _mm_set1_epi64x(0x8000000000000000);

            const auto __left_signed = _mm_xor_si128(__intrin_bitcast<__m128i>(__left), __sign_64bit);
            const auto __right_signed = _mm_xor_si128(__intrin_bitcast<__m128i>(__right), __sign_64bit);

            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi64(__left_signed, __right_signed));
        }
        else {
            return _Simd_greater<arch::ISA::SSE2, 128, _DesiredType_>()(__left, __right);
        }
    }
};

template <class _DesiredType_>
struct _Simd_greater<arch::ISA::AVX2, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX2, 256, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_>
struct _Simd_greater<arch::ISA::AVX512F, 512, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX512F, 512, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_>
struct _Simd_greater<arch::ISA::AVX512BW, 512, _DesiredType_>:
    _Simd_greater<arch::ISA::AVX512F, 512, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX512BW, 512, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_> 
struct _Simd_greater<arch::ISA::AVX512VLF, 256, _DesiredType_>:
    _Simd_greater<arch::ISA::AVX2, 256, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX512VLF, 256, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_> 
struct _Simd_greater<arch::ISA::AVX512VLBW, 256, _DesiredType_>:
    _Simd_greater<arch::ISA::AVX512VLF, 256, _DesiredType_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX512VLBW, 256, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_> 
struct _Simd_greater<arch::ISA::AVX512VLF, 128, _DesiredType_>:
    _Simd_greater<arch::ISA::AVX2, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX512VLF, 128, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_> 
struct _Simd_greater<arch::ISA::AVX512VLBW, 128, _DesiredType_>:
    _Simd_greater<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Simd_less<arch::ISA::AVX512VLBW, 128, _DesiredType_>()(__right, __left);
    }
};

template <class _DesiredType_> struct _Simd_greater<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_greater<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_greater<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::SSE41, 128, _DesiredType_> : _Simd_greater<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_greater<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_greater<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_greater<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_greater<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_greater<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_greater<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_greater<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
