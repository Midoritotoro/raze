#pragma once 

#include <src/raze/datapar/bitwise/ToVector.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class       _IntrinType_,
    class		_DesiredType_>
struct _Simd_index_mask_to_vector;

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_> {
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 2)
            return _Simd_to_vector<arch::ISA::SSE2, 128, _IntrinType_, int8>()(__mask);
        else
            return _Simd_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_> {
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 2)
            return _Simd_to_vector<arch::ISA::AVX2, 256, _IntrinType_, int8>()(__mask);
        else
            return _Simd_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_> {
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (sizeof(_DesiredType_) == 8 || sizeof(_DesiredType_) == 4)
            return _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>()(__mask);
        else
            return _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, int8>()(__mask);
    }
};

template <class _IntrinType_, class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, int8>()(__mask);
    }
};

template <class _IntrinType_, class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512BWDQ, 512, _IntrinType_, _DesiredType_>: 
     _Simd_index_mask_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_>
{};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512DQ, 512, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512DQ, 512, _IntrinType_, int8>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLDQ, 256, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512VLDQ, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512VLF, 128, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLDQ, 128, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512VLF, 128, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512VLDQ, 128, _IntrinType_, int8>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_, int8>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_, _DesiredType_> :
    _Simd_index_mask_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        return _Simd_to_vector<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::SSE3, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::SSSE3, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::SSE3, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::SSE41, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::SSSE3, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::SSE42, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::SSE41, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX2, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::SSE42, 128, _IntrinType_, _DesiredType_> {};

template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI, 512, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI2, 512, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI, 512, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512BWDQ, 512, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI2DQ, 512, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_, _DesiredType_> {};

template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVL, 256, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI2VL, 256, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVL, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI2VLDQ, 256, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_, _DesiredType_> {};

template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VLF, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX2, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVL, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI2VL, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVL, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_index_mask_to_vector<arch::ISA::AVX512VBMI2VLDQ, 128, _IntrinType_, _DesiredType_> : _Simd_index_mask_to_vector<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
