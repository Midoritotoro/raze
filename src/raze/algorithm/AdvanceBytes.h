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

template <class T, class Offset>
raze_always_inline void rewind_bytes(T*& target, Offset offset) noexcept {
    target = reinterpret_cast<T*>(const_cast<u8*>(reinterpret_cast<const volatile u8*>(target)) - offset);
}

template <class T, class Offset>
raze_always_inline void rewind_bytes(const T*& target, Offset offset) noexcept {
    target = reinterpret_cast<const T*>(const_cast<const u8*>(
        reinterpret_cast<const volatile unsigned char*>(target)) - offset);
}

template <class T, class Offset>
raze_always_inline void advance_bytes(T*& target, Offset offset) noexcept {
    target = reinterpret_cast<T*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(target)) + offset);
}

template <class T, class Offset>
raze_always_inline void advance_bytes(const T*& target, Offset offset) noexcept {
    target = reinterpret_cast<const T*>(const_cast<const unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(target)) + offset);
}

template <class T1, class T2, class Offset>
raze_always_inline void advance_bytes(T1*& target1, T2*& target2, Offset offset) noexcept {
    advance_bytes(target1, offset);
    advance_bytes(target2, offset);
}

template <class T1, class T2, class Offset>
raze_always_inline void advance_bytes(const T1*& target1, const T2*& target2, Offset offset) noexcept {
    advance_bytes(target1, offset);
    advance_bytes(target2, offset);
}

raze_always_inline sizetype byte_length(const volatile void* first, const volatile void* last) noexcept {
    return static_cast<sizetype>(const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(last)) - 
        const_cast<const unsigned char*>(reinterpret_cast<const volatile unsigned char*>(first)));
}

template <class InIt> 
constexpr inline bool is_nothrow_distance_v = std::random_access_iterator<InIt>
    || std::bool_constant<noexcept(std::declval<std::remove_reference_t<InIt>&>()++)>::value;

template <class InIt, class DiffType = std::iter_difference_t<InIt>>
raze_nodiscard raze_always_inline constexpr std::iter_difference_t<InIt> distance(
    InIt first, InIt last) noexcept(is_nothrow_distance_v<InIt>)
{
    if constexpr (std::random_access_iterator<InIt>) {
        return static_cast<DiffType>(last - first);
    }
    else {
        verify_range(first, last);

        auto first_unwrapped = uiter(first);
        const auto last_unwrapped = uiter(last);

        auto distance = DiffType(0);

        for (; first_unwrapped != last_unwrapped; ++first_unwrapped)
            ++distance;

        return distance;
    }
}

template <class T, class Offset>
constexpr raze_always_inline T* bytes_pointer_offset(T* target, Offset offset) noexcept {
    return reinterpret_cast<T*>(const_cast<unsigned char*>(
        reinterpret_cast<const volatile unsigned char*>(target)) + offset);
}

template <std::ranges::contiguous_range Range>
constexpr std::integral_constant<sizetype, range_constexpr_size<Range>() * sizeof(std::ranges::range_value_t<Range>)>
bytes_distance(const options::as<Range>&) noexcept requires(constexpr_sized_range<Range>) {
    return std::integral_constant<sizetype, range_constexpr_size<Range>() * sizeof(std::ranges::range_value_t<Range>)>{};
}

template <std::ranges::contiguous_range Range>
constexpr auto bytes_distance(Range&& r) noexcept(noexcept(std::ranges::size(r))) {
    return std::ranges::size(r) * sizeof(std::ranges::range_value_t<Range>);
}

template <std::contiguous_iterator It, std::sentinel_for<It> Sent>
constexpr auto bytes_distance(It it, Sent sent) noexcept(noexcept(std::ranges::distance(it, sent))) {
    return std::ranges::distance(it, sent) * sizeof(std::iter_value_t<It>);
}

template <std::contiguous_iterator It>
constexpr auto bytes_distance(It first, It last) noexcept(noexcept(algorithm::distance(first, last))) {
    return algorithm::distance(first, last) * sizeof(std::iter_value_t<It>);
}

__RAZE_ALGORITHM_NAMESPACE_END
