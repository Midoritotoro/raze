#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/type_traits/SimdTypeCheck.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/Concepts.h>


__RAZE_VX_NAMESPACE_BEGIN

//template <class _Type_>
//concept __cast_to_type = simd_type<_Type_> || arithmetic_type<_Type_>;
//
//template <class _To_, class _From_, bool _IsSimd_ = __is_valid_simd_v<_To_>>
//struct __rebind_vector_element {
//    using type = void;
//};
//
//template <class _To_, class _From_>
//struct __rebind_vector_element<_To_, _From_, false> {
//    using type = simd<_To_, typename _From_::abi_type>;
//};
//
//template <class _To_, class _From_>
//struct __rebind_vector_element<_To_, _From_, true> {
//    using type = _To_;
//};
//
//template <class _To_, class _From_>
//using __revind_vector_element_type = typename __rebind_vector_element<_To_, _From_>::type;
//
//template <__cast_to_type _ToType_, simd_type _FromType_>
//raze_nodiscard raze_always_inline _ToType_ simd_cast(_FromType_ __from) noexcept {
//    using _ToIntrinType = __unwrapped_vector_type<__revind_vector_element_type<_ToType_, _FromType_>>;
//
//    if constexpr (__is_valid_simd_v<_FromType_>)
//        return __as<_ToIntrinType>(static_cast<typename _FromType_::vector_type>(__from));
//    else
//        return __as<_ToIntrinType>(__from);
//}

__RAZE_VX_NAMESPACE_END
