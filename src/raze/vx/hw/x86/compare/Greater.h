#pragma once 

#include <src/raze/vx/compare/Less.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_Type_>
struct _Greater;

template <class _Type_>
struct _Greater<arch::ISA::SSE2, 128, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::SSE2, 128, _Type_>()(__right, __left);
    }
};

template <class _Type_>
struct _Greater<arch::ISA::SSE42, 128, _Type_>:
    _Greater<arch::ISA::SSE41, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_>) {
            return __as<_IntrinType_>(_mm_cmpgt_epi64(
                __as<__m128i>(__left), __as<__m128i>(__right)));
        }
        else if constexpr (__is_epu64_v<_Type_>) {
            const auto __sign_64bit = _mm_set1_epi64x(0x8000000000000000);

            const auto __left_signed = _mm_xor_si128(__as<__m128i>(__left), __sign_64bit);
            const auto __right_signed = _mm_xor_si128(__as<__m128i>(__right), __sign_64bit);

            return __as<_IntrinType_>(_mm_cmpgt_epi64(__left_signed, __right_signed));
        }
        else {
            return _Greater<arch::ISA::SSE2, 128, _Type_>()(__left, __right);
        }
    }
};

template <class _Type_>
struct _Greater<arch::ISA::AVX, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX, 256, _Type_>()(__right, __left);
    }
};

template <class _Type_>
struct _Greater<arch::ISA::AVX2, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX2, 256, _Type_>()(__right, __left);
    }
};

template <class _Type_>
struct _Greater<arch::ISA::AVX512F, 512, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX512F, 512, _Type_>()(__right, __left);
    }
};

template <class _Type_>
struct _Greater<arch::ISA::AVX512BW, 512, _Type_>:
    _Greater<arch::ISA::AVX512F, 512, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX512BW, 512, _Type_>()(__right, __left);
    }
};

template <class _Type_> 
struct _Greater<arch::ISA::AVX512VLF, 256, _Type_>:
    _Greater<arch::ISA::AVX2, 256, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX512VLF, 256, _Type_>()(__right, __left);
    }
};

template <class _Type_> 
struct _Greater<arch::ISA::AVX512VLBW, 256, _Type_>:
    _Greater<arch::ISA::AVX512VLF, 256, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX512VLBW, 256, _Type_>()(__right, __left);
    }
};

template <class _Type_> 
struct _Greater<arch::ISA::AVX512VLF, 128, _Type_>:
    _Greater<arch::ISA::AVX2, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX512VLF, 128, _Type_>()(__right, __left);
    }
};

template <class _Type_> 
struct _Greater<arch::ISA::AVX512VLBW, 128, _Type_>:
    _Greater<arch::ISA::AVX512VLF, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) raze_const_operator noexcept
    {
        return _Less<arch::ISA::AVX512VLBW, 128, _Type_>()(__right, __left);
    }
};

template <class _Type_> struct _Greater<arch::ISA::SSE3, 128, _Type_> : _Greater<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::SSSE3, 128, _Type_> : _Greater<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::SSE41, 128, _Type_> : _Greater<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX, 128, _Type_> : _Greater<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::FMA3, 128, _Type_> : _Greater<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX2, 128, _Type_> : _Greater<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX2FMA3, 128, _Type_> : _Greater<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Greater<arch::ISA::AVX512DQ, 512, _Type_> : _Greater<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512BWDQ, 512, _Type_> : _Greater<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI, 512, _Type_> : _Greater<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI2, 512, _Type_> : _Greater<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Greater<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Greater<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Greater<arch::ISA::FMA3, 256, _Type_> : _Greater<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX2FMA3, 256, _Type_> : _Greater<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VLDQ, 256, _Type_> : _Greater<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Greater<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Greater<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Greater<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Greater<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Greater<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Greater<arch::ISA::AVX512VLDQ, 128, _Type_> : _Greater<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Greater<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Greater<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Greater<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Greater<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Greater<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Greater<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
