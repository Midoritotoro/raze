#pragma once

#include <raze/arch/CpuFeature.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    class       _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v = 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::SSE2, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::SSE3, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::SSSE3, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::SSE41, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::SSE42, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX2, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX2, 256, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512F, 512, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512BW, 512, _DesiredType_> = 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512BWDQ, 512, _DesiredType_> = 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512VLF, 256, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512VLBW, 256, _DesiredType_> = 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512VLDQ, 256, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v< arch::ISA::AVX512VLF, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512VLBW, 128, _DesiredType_> = 1;

template <class _DesiredType_>
inline constexpr sizetype __simd_index_mask_divisor_v<arch::ISA::AVX512VLDQ, 128, _DesiredType_> = (sizeof(_DesiredType_) == 2) ? 2 : 1;

template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    class       _DesiredType_>
constexpr inline auto __simd_index_mask_divisor = __simd_index_mask_divisor_v<_ISA_, _Width_, _DesiredType_>;

__RAZE_DATAPAR_NAMESPACE_END
