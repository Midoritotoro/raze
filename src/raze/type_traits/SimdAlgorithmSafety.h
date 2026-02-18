#pragma once 

#include <src/raze/type_traits/IteratorCheck.h>

__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <
    class _FirstType_,
    class _SecondType_>
struct __is_safe_find_with_object_pointers: 
    std::false_type
{};

template <
    class _FirstType_,
    class _SecondType_>
struct __is_safe_find_with_object_pointers<_FirstType_*, _SecondType_*>:
    std::conjunction<
        std::disjunction<
            std::is_object<_FirstType_>, 
            std::is_void<_FirstType_>>,
        std::disjunction<
            std::is_object<_SecondType_>, 
            std::is_void<_SecondType_>>,
        std::disjunction<
            std::is_same<
                std::remove_cv_t<_FirstType_>, 
                std::remove_cv_t<_SecondType_>>,
            std::is_void<_FirstType_>,
            std::is_void<_SecondType_>>
    >
{};

template <
    class _Type_,
    class _Element_>
constexpr bool __is_vectorized_algorithm_element_safe_v = std::disjunction_v<
#ifdef __cpp_lib_byte
    std::conjunction<
        std::is_same<_Type_, std::byte>,
        std::is_same<_Element_, std::byte>>,
#endif // defined(__cpp_lib_byte)
    std::conjunction<
        std::is_integral<_Type_>,
        std::is_integral<_Element_>>,
    std::conjunction<
        std::is_floating_point<_Type_>,
        std::is_floating_point<_Element_>>,
    std::conjunction<
        std::is_pointer<_Type_>,
        std::is_same<_Type_, _Element_>>,
    std::conjunction<
        std::is_same<_Type_, std::nullptr_t>,
        std::is_pointer<_Element_>>,
    __is_safe_find_with_object_pointers<_Type_, _Element_>>;

template <
    class _Iterator_,
    class _Type_>
constexpr bool __is_vectorized_find_algorithm_safe_v =
    is_iterator_contiguous_v<_Iterator_>    && 
    !is_iterator_volatile_v<_Iterator_>     &&
    __is_vectorized_algorithm_element_safe_v<_Type_, iterator_value_type<_Iterator_>>;

__RAZE_TYPE_TRAITS_NAMESPACE_END
