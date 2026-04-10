#pragma once 

#include <raze/arch/CpuFeature.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    class       _Type_>
inline constexpr bool __is_native_mask_load_supported_v = false;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VLF, 128, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VLBW, 128, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX, 128, _Type_> = (sizeof(_Type_) >= 4) && 
        std::is_floating_point_v<_Type_>;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::FMA3, 128, _Type_> = (sizeof(_Type_) >= 4) && 
        std::is_floating_point_v<_Type_>;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX2, 128, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX2FMA3, 128, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX, 256, _Type_> = (sizeof(_Type_) >= 4) && 
        std::is_floating_point_v<_Type_>;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::FMA3, 256, _Type_> = (sizeof(_Type_) >= 4) && 
        std::is_floating_point_v<_Type_>;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX2, 256, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX2FMA3, 256, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VLF, 256, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VLBW, 256, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VLBWDQ, 256, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512F, 512, _Type_> = sizeof(_Type_) >= 4;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512BW, 512, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512BWDQ, 512, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI, 512, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI2, 512, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<arch::ISA::AVX512VBMIDQ, 512, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI2DQ, 512, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMIVL, 256, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI2VL, 256, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMIVLDQ, 256, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMIVL, 128, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI2VL, 128, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMIVLDQ, 128, _Type_> = true;

template <class _Type_>
inline constexpr bool __is_native_mask_load_supported_v<
    arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> = true;

__RAZE_VX_NAMESPACE_END
