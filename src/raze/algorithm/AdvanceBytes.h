#pragma once 

#include <raze/compatibility/Inline.h>
#include <raze/RazeNamespace.h>

#include <src/raze/type_traits/IteratorCheck.h>
#include <raze/Types.h>

#include <src/raze/utility/Assert.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_, class _Integral_>
raze_always_inline void __rewind_bytes(_Type_*& __target, _Integral_ __offset) noexcept {
    __target = reinterpret_cast<_Type_*>(const_cast<u8*>(
        reinterpret_cast<const volatile u8*>(__target)) - __offset);
}

template <class _Type_, class _Integral_>
raze_always_inline void __rewind_bytes(const _Type_*& __target, _Integral_ __offset) noexcept {
    __target = reinterpret_cast<const _Type_*>(const_cast<const u8*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) - __offset);
}

template <class _Type_, class _Integral_>
raze_always_inline void __advance_bytes(_Type_*& __target, _Integral_ __offset) noexcept {
    __target = reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) + __offset);
}

template <class _Type_, class _Integral_>
raze_always_inline void __advance_bytes(const _Type_*& __target, _Integral_ __offset) noexcept {
    __target = reinterpret_cast<const _Type_*>(const_cast<const unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) + __offset);
}

template <class _Type1_, class _Type2_, class _Integral_>
raze_always_inline void __advance_bytes(_Type1_*& __target1, _Type2_*& __target2, _Integral_ __offset) noexcept {
    __advance_bytes(__target1, __offset);
    __advance_bytes(__target2, __offset);
}

template <class _Type1_, class _Type2_, class _Integral_>
raze_always_inline void __advance_bytes(const _Type1_*& __target1, const _Type2_*& __target2, _Integral_ __offset) noexcept {
    __advance_bytes(__target1, __offset);
    __advance_bytes(__target2, __offset);
}

raze_always_inline sizetype __byte_length(const volatile void* __first, const volatile void* __last) noexcept {
    return static_cast<sizetype>(const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(__last)) - 
        const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(__first)));
}

template <class _InputIterator_> 
constexpr inline bool __is_nothrow_distance_v = type_traits::is_iterator_random_ranges_v<_InputIterator_> 
    || std::bool_constant<noexcept(std::declval<std::remove_reference_t<_InputIterator_>&>()++)>::value;

template <class _InputIterator_, class _DifferenceType_ = type_traits::iterator_difference_type<_InputIterator_>>
raze_nodiscard raze_always_inline constexpr type_traits::iterator_difference_type<_InputIterator_> distance(
    _InputIterator_ __first, _InputIterator_ __last) noexcept(__is_nothrow_distance_v<_InputIterator_>)
{
    if constexpr (type_traits::is_iterator_random_ranges_v<_InputIterator_>) {
        return static_cast<_DifferenceType_>(__last - __first);
    }
    else {
        __verify_range(__first, __last);

        auto __first_unwrapped = __unwrap_iterator(__first);
        const auto __last_unwrapped = __unwrap_iterator(__last);

        auto __distance = _DifferenceType_(0);

        for (; __first_unwrapped != __last_unwrapped; ++__first_unwrapped)
            ++__distance;

        return __distance;
    }
}

template <class _Type_, class _Integral_>
constexpr raze_always_inline _Type_* __bytes_pointer_offset(
    _Type_* __target, _Integral_ __offset) noexcept
{
    return reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(__target)) + __offset);
}

__RAZE_ALGORITHM_NAMESPACE_END
