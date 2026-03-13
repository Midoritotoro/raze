#pragma once 

#include <raze/arch/CpuFeature.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    sizetype    _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v = false;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VLF, 128, _TypeSize_> = (_TypeSize_ == 4) || (_TypeSize_ == 8);

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VLBW, 128, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX2, 256, _TypeSize_> = (_TypeSize_ == 4) || (_TypeSize_ == 8);

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VLF, 256, _TypeSize_> = (_TypeSize_ == 4) || (_TypeSize_ == 8);

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VLBW, 256, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512F, 512, _TypeSize_> = (_TypeSize_ == 4) || (_TypeSize_ == 8);

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512BW, 512, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512BWDQ, 512, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI, 512, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI2, 512, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMIDQ, 512, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI2DQ, 512, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMIVL, 256, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI2VL, 256, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMIVLDQ, 256, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI2VLDQ, 256, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMIVL, 128, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI2VL, 128, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMIVLDQ, 128, _TypeSize_> = true;

template <sizetype _TypeSize_>
inline constexpr bool __is_native_mask_store_supported_v<arch::ISA::AVX512VBMI2VLDQ, 128, _TypeSize_> = true;

__RAZE_DATAPAR_NAMESPACE_END
