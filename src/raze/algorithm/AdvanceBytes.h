#pragma once 

#include <raze/compatibility/Inline.h>
#include <raze/RazeNamespace.h>

#include <src/raze/traits/IteratorCheck.h>
#include <raze/Types.h>

#include <src/raze/utility/Assert.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/options/As.h>


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
constexpr inline bool __is_nothrow_distance_v = std::random_access_iterator<_InputIterator_> 
    || std::bool_constant<noexcept(std::declval<std::remove_reference_t<_InputIterator_>&>()++)>::value;

template <class _InputIterator_, class _DifferenceType_ = std::iter_difference_t<_InputIterator_>>
raze_nodiscard raze_always_inline constexpr std::iter_difference_t<_InputIterator_> distance(
    _InputIterator_ __first, _InputIterator_ __last) noexcept(__is_nothrow_distance_v<_InputIterator_>)
{
    if constexpr (std::random_access_iterator<_InputIterator_>) {
        return static_cast<_DifferenceType_>(__last - __first);
    }
    else {
        __verify_range(__first, __last);

        auto __first_unwrapped = __uiter(__first);
        const auto __last_unwrapped = __uiter(__last);

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

template <std::ranges::contiguous_range _Range_>
constexpr std::integral_constant<sizetype, __range_constexpr_size<_Range_>() * sizeof(std::ranges::range_value_t<_Range_>)>
__bytes_distance(const options::as<_Range_>&) noexcept requires(constexpr_sized_range<_Range_>)
{
    return std::integral_constant<sizetype, __range_constexpr_size<_Range_>() * sizeof(std::ranges::range_value_t<_Range_>)>{};
}

template <std::ranges::contiguous_range _Range_>
constexpr auto __bytes_distance(_Range_&& __r) noexcept {
    return std::ranges::size(__r) * sizeof(std::ranges::range_value_t<_Range_>);
}

template <std::contiguous_iterator _It_, std::sentinel_for<_It_> _Sent_>
constexpr auto __bytes_distance(_It_ __it, _Sent_ __sent) noexcept {
    return std::ranges::distance(__it, __sent) * sizeof(std::iter_value_t<_It_>);
}

template <std::contiguous_iterator _It_>
constexpr auto __bytes_distance(_It_ __first, _It_ __last) noexcept {
    return algorithm::distance(__first, __last) * sizeof(std::iter_value_t<_It_>);
}

__RAZE_ALGORITHM_NAMESPACE_END
