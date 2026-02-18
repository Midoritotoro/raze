#pragma once 

#include <raze/math/IntegralTypesConversions.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _InputIterator_,
    class _Type_>
__simd_nodiscard_inline constexpr bool __is_comparable(const _Type_& __value) noexcept {
    constexpr auto __is_safe_comparable = std::disjunction_v<
#if defined(__cpp_lib_byte)
        std::is_same<_Type_, std::byte>,
#endif // defined(__cpp_lib_byte)
        std::is_same<_Type_, bool>, std::is_pointer<_Type_>, std::is_same<_Type_, std::nullptr_t>>;

    if constexpr (__is_safe_comparable) {
        return true;
    } 
    else {
        using _ElementType_ = type_traits::iterator_value_type<_InputIterator_>;
        static_assert(std::is_integral_v<_ElementType_> && std::is_integral_v<_Type_>);

        if constexpr (std::is_same_v<_ElementType_, bool>) {
            return __value == true || __value == false;
        } 
		else if constexpr (std::is_signed_v<_ElementType_>) {
            constexpr auto __minimum = math::__minimum_integral_limit<_ElementType_>();
            constexpr auto __maximum = math::__maximum_integral_limit<_ElementType_>();

            if constexpr (std::is_signed_v<_Type_>) {
                return __minimum <= __value && __value <= __maximum;
            } 
			else {
                if constexpr (_ElementType_{-1} == static_cast<_Type_>(-1)) 
                    return __value <= __maximum || static_cast<_Type_>(__minimum) <= __value;
                else
                    return __value <= __maximum;
            }
        } else {
            constexpr auto __maximum = math::__maximum_integral_limit<_ElementType_>();

            if constexpr (std::is_unsigned_v<_Type_>) {
                return __value <= __maximum;
            } 
			else {
                if constexpr (static_cast<_Type_>(-1) == static_cast<_ElementType_>(-1))
                    return __value <= __maximum;
                else
                    return 0 <= __value && __value <= __maximum;
            }
        }
    }
}

__RAZE_DATAPAR_NAMESPACE_END
