#pragma once 

#include <raze/compatibility/Inline.h>
#include <raze/RazeNamespace.h>

#include <src/raze/type_traits/IteratorCheck.h>
#include <raze/Types.h>

#include <src/raze/utility/Assert.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    typename    _Type_,
    class       _Integral_>
raze_always_inline void __rewind_bytes(
    _Type_*&    __target,
    _Integral_  __offset) noexcept
{
    __target = reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) - __offset);
}

template <
    typename    _Type_,
    class       _Integral_>
raze_always_inline void __rewind_bytes(
    const _Type_*&  __target,
    _Integral_      __offset) noexcept
{
    __target = reinterpret_cast<const _Type_*>(const_cast<const unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) - __offset);
}

template <
    typename    _Type_,
    class       _Integral_>
raze_always_inline void __advance_bytes(
    _Type_*&    __target,
    _Integral_  __offset) noexcept
{
    __target = reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) + __offset);
}

template <
    typename    _Type_,
    class       _Integral_>
raze_always_inline void __advance_bytes(
    const _Type_*&  __target,
    _Integral_      __offset) noexcept
{
    __target = reinterpret_cast<const _Type_*>(const_cast<const unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) + __offset);
}


raze_always_inline sizetype __byte_length(
    const volatile void* __first,
    const volatile void* __last) noexcept
{
    return static_cast<sizetype>(
        const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(__last)) - 
        const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(__first)));
}


template <class _ContiguousIterator_>
constexpr inline type_traits::iterator_difference_type<_ContiguousIterator_> __iterators_difference(
    const _ContiguousIterator_& __first,
    const _ContiguousIterator_& __last) noexcept
{
    using _DifferenceType_ = type_traits::iterator_difference_type<_ContiguousIterator_>;

    if constexpr (std::is_pointer_v<_ContiguousIterator_> || type_traits::is_iterator_random_ranges_v<_ContiguousIterator_>)
        return static_cast<_DifferenceType_>(__last - __first);

    const auto __first_address  = std::to_address(__first);
    const auto __second_address = std::to_address(__last);

    using _IteratorValueType_ = type_traits::iterator_value_type<_ContiguousIterator_>;

    const auto __first_iterator_address = const_cast<const _IteratorValueType_*>(
        reinterpret_cast<const volatile _IteratorValueType_*>(__first_address));

    const auto __last_iterator_address = const_cast<const _IteratorValueType_*>(
        reinterpret_cast<const volatile _IteratorValueType_*>(__second_address));

    return static_cast<_DifferenceType_>(__last_iterator_address - __first_iterator_address);
}

template <class _InputIterator_> 
constexpr inline bool __is_nothrow_distance_v = type_traits::is_iterator_random_ranges_v<_InputIterator_> 
    || std::bool_constant<noexcept(std::declval<std::remove_reference_t<_InputIterator_>&>()++)>::value;

template <
    class _InputIterator_,
    class _DifferenceType_ = type_traits::iterator_difference_type<_InputIterator_>>
raze_nodiscard raze_always_inline constexpr type_traits::iterator_difference_type<_InputIterator_> distance(
    _InputIterator_ __first,
    _InputIterator_ __last) noexcept(__is_nothrow_distance_v<_InputIterator_>)
{
    if constexpr (type_traits::is_iterator_random_ranges_v<_InputIterator_>) {
        return static_cast<_DifferenceType_>(__last - __first);
    }
    else {
        __verify_range(__first, __last);

        auto __first_unwrapped        = __unwrap_iterator(__first);
        const auto __last_unwrapped   = __unwrap_iterator(__last);

        auto __distance = _DifferenceType_(0);

        for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
            ++__distance;

        return __distance;
    }
}

template <
    class _Char_, 
    class _UnsignedIntegralType_>
raze_nodiscard _Char_* __unsigned_integral_to_buffer(
    _Char_*                 __end,
    _UnsignedIntegralType_  __value) noexcept
{ 
    static_assert(std::is_unsigned_v<_UnsignedIntegralType_>);

    auto __truncated = __value;

    do {
        *--__end = static_cast<_Char_>('0' + __truncated % 10);
        __truncated /= 10;
    } while (__truncated != 0);

    return __end;
}

template <
    typename    _Type_,
    class       _Integral_>
raze_always_inline _Type_* __bytes_pointer_offset(
    _Type_*   __target,
    _Integral_      __offset) noexcept
{
    return reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) + __offset);
}


template <
    typename    _Type_,
    class       _Integral_>
raze_always_inline const _Type_* __pointer_offset(
    const _Type_*   __target,
    _Integral_      __offset) noexcept
{
    return __target + __offset;
}

__RAZE_ALGORITHM_NAMESPACE_END
