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
raze_always_inline void rewind_bytes(_Type_*& target, _Integral_ offset) noexcept {
    target = reinterpret_cast<_Type_*>(const_cast<u8*>(
        reinterpret_cast<const volatile u8*>(target)) - offset);
}

template <class _Type_, class _Integral_>
raze_always_inline void rewind_bytes(const _Type_*& target, _Integral_ offset) noexcept {
    target = reinterpret_cast<const _Type_*>(const_cast<const u8*>(
        reinterpret_cast<const volatile unsigned char*>(target)) - offset);
}

template <class _Type_, class _Integral_>
raze_always_inline void advance_bytes(_Type_*& target, _Integral_ offset) noexcept {
    target = reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(target)) + offset);
}

template <class _Type_, class _Integral_>
raze_always_inline void advance_bytes(const _Type_*& target, _Integral_ offset) noexcept {
    target = reinterpret_cast<const _Type_*>(const_cast<const unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(target)) + offset);
}

template <class _Type1_, class _Type2_, class _Integral_>
raze_always_inline void advance_bytes(_Type1_*& target1, _Type2_*& target2, _Integral_ offset) noexcept {
    advance_bytes(target1, offset);
    advance_bytes(target2, offset);
}

template <class _Type1_, class _Type2_, class _Integral_>
raze_always_inline void advance_bytes(const _Type1_*& target1, const _Type2_*& target2, _Integral_ offset) noexcept {
    advance_bytes(target1, offset);
    advance_bytes(target2, offset);
}

raze_always_inline sizetype byte_length(const volatile void* first, const volatile void* last) noexcept {
    return static_cast<sizetype>(const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(last)) - 
        const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(first)));
}

template <class _InputIterator_> 
constexpr inline bool is_nothrow_distance_v = std::random_access_iterator<_InputIterator_> 
    || std::bool_constant<noexcept(std::declval<std::remove_reference_t<_InputIterator_>&>()++)>::value;

template <class _InputIterator_, class _DifferenceType_ = std::iter_difference_t<_InputIterator_>>
raze_nodiscard raze_always_inline constexpr std::iter_difference_t<_InputIterator_> distance(
    _InputIterator_ first, _InputIterator_ last) noexcept(is_nothrow_distance_v<_InputIterator_>)
{
    if constexpr (std::random_access_iterator<_InputIterator_>) {
        return static_cast<_DifferenceType_>(last - first);
    }
    else {
        verify_range(first, last);

        auto first_unwrapped = uiter(first);
        const auto last_unwrapped = uiter(last);

        auto distance = _DifferenceType_(0);

        for (; first_unwrapped != last_unwrapped; ++first_unwrapped)
            ++distance;

        return distance;
    }
}

template <class _Type_, class _Integral_>
constexpr raze_always_inline _Type_* bytes_pointer_offset(
    _Type_* target, _Integral_ offset) noexcept
{
    return reinterpret_cast<_Type_*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(target)) + offset);
}

template <std::ranges::contiguous_range Range>
constexpr std::integral_constant<sizetype, range_constexpr_size<Range>() * sizeof(std::ranges::range_value_t<Range>)>
bytes_distance(const options::as<Range>&) noexcept requires(constexpr_sized_range<Range>)
{
    return std::integral_constant<sizetype, range_constexpr_size<Range>() * sizeof(std::ranges::range_value_t<Range>)>{};
}

template <std::ranges::contiguous_range Range>
constexpr auto bytes_distance(Range&& r) noexcept {
    return std::ranges::size(r) * sizeof(std::ranges::range_value_t<Range>);
}

template <std::contiguous_iterator _It_, std::sentinel_for<_It_> _Sent_>
constexpr auto bytes_distance(_It_ it, _Sent_ sent) noexcept {
    return std::ranges::distance(it, sent) * sizeof(std::iter_value_t<_It_>);
}

template <std::contiguous_iterator _It_>
constexpr auto bytes_distance(_It_ first, _It_ last) noexcept {
    return algorithm::distance(first, last) * sizeof(std::iter_value_t<_It_>);
}

__RAZE_ALGORITHM_NAMESPACE_END
