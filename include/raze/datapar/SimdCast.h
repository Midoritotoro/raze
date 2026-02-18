#pragma once 

#include <src/raze/datapar/IntrinBitcast.h>
#include <src/raze/type_traits/SimdTypeCheck.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _VectorType_>
__simd_nodiscard_inline __unwrapped_vector_type<_VectorType_> __simd_unwrap(_VectorType_ __vector) noexcept;

template <class _MaskType_, std::enable_if_t<__is_valid_simd_v<_MaskType_> ||
    __is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_> || __is_simd_mask_v<_MaskType_>, int> = 0>
__simd_nodiscard_inline auto __simd_unwrap_mask(_MaskType_ __mask) noexcept;

template <
    class _ToType_,
    class _FromType_,
    std::enable_if_t<(__is_valid_simd_v<_ToType_> || __is_intrin_type_v<_ToType_> ||
        type_traits::__is_vector_type_supported_v<_ToType_>) &&
    (__is_valid_simd_v<_FromType_> || __is_intrin_type_v<_FromType_>), int> = 0>
__simd_nodiscard_inline __rebind_vector_element_type<_ToType_, _FromType_> simd_cast(_FromType_ __from) noexcept;

template <
    arch::ISA	_ToSimdGeneration_,
    class               _FromVector_,
    std::enable_if_t<__is_valid_simd_v<_FromVector_> || __is_intrin_type_v<_FromVector_>, int> = 0>
__simd_nodiscard_inline __rebind_vector_generation_type<_ToSimdGeneration_,
    __vector_element_type<_FromVector_>, _FromVector_> simd_cast(_FromVector_ __from) noexcept;

template <
    arch::ISA	_ToSimdGeneration_,
    class               _ToElementType_,
    class               _FromVector_,
    std::enable_if_t<__is_valid_simd_v<_FromVector_> || __is_intrin_type_v<_FromVector_>, int> = 0>
__simd_nodiscard_inline __rebind_vector_generation_type<_ToSimdGeneration_,
    _ToElementType_, _FromVector_> simd_cast(_FromVector_ __from) noexcept;

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/SimdCast.inl>