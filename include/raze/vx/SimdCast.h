#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/type_traits/SimdTypeCheck.h>
#include <raze/vx/Abi.h>


__RAZE_VX_NAMESPACE_BEGIN
//
//template <
//    class _RebindType_,
//    class _VectorType_,
//    bool _IsBasicSimd_,
//    bool _IsIntrin_>
//struct __rebind_vector_element_t {
//    using type = void;
//};
//
//template <
//    class _RebindType_,
//    class _VectorType_>
//struct __rebind_vector_element_t<_RebindType_, _VectorType_, false, true> {
//    using type = std::conditional_t<__is_intrin_type_v<_RebindType_> || __is_valid_simd_v<_RebindType_>, _RebindType_,
//        std::conditional_t<sizeof(_VectorType_) == 64,
//        type_traits::__deduce_simd_vector_type<_RebindType_, 512>,
//        std::conditional_t<sizeof(_VectorType_) == 32,
//        type_traits::__deduce_simd_vector_type<_RebindType_, 256>,
//        std::conditional_t<sizeof(_VectorType_) == 16,
//        type_traits::__deduce_simd_vector_type<_RebindType_, 128>, void>>>>;
//};
//
//template <
//    class _RebindType_,
//    class _VectorType_>
//struct __rebind_vector_element_t<_RebindType_, _VectorType_, true, false> {
//    using type = std::conditional_t<__is_intrin_type_v<_RebindType_> || __is_valid_simd_v<_RebindType_>,
//        _RebindType_, simd<_RebindType_, runtime_abi<_VectorType_::__isa, _VectorType_::__width>>>;
//};
//
//template <
//    arch::ISA	_ToSimdGeneration_,
//    class               _RebindType_,
//    class               _VectorType_,
//    bool                _IsBasicSimd_,
//    bool                _IsIntrin_>
//struct __rebind_vector_generation_t {
//    using type = void;
//};
//
//template <
//    arch::ISA	_ToSimdGeneration_,
//    class       _RebindType_,
//    class       _VectorType_>
//struct __rebind_vector_generation_t<_ToSimdGeneration_, _RebindType_, _VectorType_, false, true> {
//    using type = type_traits::__deduce_simd_vector_type<_RebindType_, __default_width<_ToSimdGeneration_>>;
//};
//
//template <
//    arch::ISA	_ToSimdGeneration_,
//    class       _RebindType_,
//    class       _VectorType_>
//struct __rebind_vector_generation_t<_ToSimdGeneration_, _RebindType_, _VectorType_, true, false> {
//    using type = simd<_RebindType_, runtime_abi<_ToSimdGeneration_>>;
//};
//
//template <
//    class _RebindType_,
//    class _VectorType_>
//using __rebind_vector_element_type = typename __rebind_vector_element_t<_RebindType_, _VectorType_>::type;
//
//template <
//    arch::ISA	_ToSimdGeneration_,
//    class               _RebindType_,
//    class               _VectorType_>
//using __rebind_vector_generation_type = typename __rebind_vector_generation_t<_ToSimdGeneration_, _RebindType_, _VectorType_>::type;
//
//
//template <
//    class _ToType_,
//    class _FromType_>
//__simd_nodiscard_inline __rebind_vector_element_type<_ToType_, _FromType_> simd_cast(_FromType_ __from) noexcept
//    requires (__is_valid_simd_v<_ToType_> || __is_intrin_type_v<_ToType_>
//        || type_traits::__is_vector_type_supported_v<_ToType_>) 
//        && (__is_valid_simd_v<_FromType_> || __is_intrin_type_v<_FromType_>)
//{
//    using _ToIntrinType = __unwrapped_vector_type<__rebind_vector_element_type<_ToType_, _FromType_>>;
//
//    if constexpr (__is_valid_simd_v<_FromType_>)
//        return __as<_ToIntrinType>(static_cast<typename _FromType_::vector_type>(__from));
//    else
//        return __as<_ToIntrinType>(__from);
//}
//
//template <
//    arch::ISA	_ToSimdGeneration_,
//    class       _FromVector_>
//__simd_nodiscard_inline __rebind_vector_generation_type<_ToSimdGeneration_,
//    __vector_element_type<_FromVector_>, _FromVector_> simd_cast(_FromVector_ __from) noexcept
//        requires(__is_valid_simd_v<_FromVector_> || __is_intrin_type_v<_FromVector_>)
//{
//    using _ToIntrinType = __unwrapped_vector_type<__rebind_vector_generation_type<_ToSimdGeneration_,
//        __vector_element_type<_FromVector_>, _FromVector_>>;
//
//    if constexpr (__is_valid_simd_v<_FromVector_>)
//        return __as<_ToIntrinType>(static_cast<typename _FromVector_::vector_type>(__from));
//    else
//        return __as<_ToIntrinType>(__from);
//}
//
//template <
//    arch::ISA	_ToSimdGeneration_,
//    class       _ToElementType_,
//    class       _FromVector_>
//__simd_nodiscard_inline __rebind_vector_generation_type<_ToSimdGeneration_,
//    _ToElementType_, _FromVector_> simd_cast(_FromVector_ __from) noexcept
//        requires(__is_valid_simd_v<_FromVector_> || __is_intrin_type_v<_FromVector_>)
//{
//
//    using _ToIntrinType = __unwrapped_vector_type<__rebind_vector_generation_type<_ToSimdGeneration_,
//        _ToElementType_, _FromVector_>>;
//
//    if constexpr (__is_valid_simd_v<_FromVector_>)
//        return __as<_ToIntrinType>(static_cast<typename _FromVector_::vector_type>(__from));
//    else
//        return __as<_ToIntrinType>(__from);
//}

__RAZE_VX_NAMESPACE_END