#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <ranges>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class>
constexpr inline auto is_std_array_v = false;

template <class T, sizetype N>
constexpr inline auto is_std_array_v<std::array<T, N>> = true;

template <class>
constexpr inline auto is_std_span_v = false;

template <class T, sizetype N>
inline constexpr bool is_std_span_v<std::span<T, N>> = true;

template <auto> 
struct require_constant;

template <class Range>
constexpr auto range_constexpr_size() noexcept {
    using Raw = std::remove_cvref_t<Range>;

    if constexpr (std::is_bounded_array_v<Raw>) return std::extent_v<Raw>;
    else if constexpr (is_std_array_v<Raw>) return std::tuple_size_v<Raw>;
    else if constexpr (is_std_span_v<Raw>) return Raw::extent;
    else if constexpr (std::ranges::sized_range<Raw> && requires { typename require_constant<Raw::size()>; }) return Raw::size();
    else return std::dynamic_extent;
}

template <class Range>
concept constexpr_sized_range = std::ranges::range<std::remove_cvref_t<Range>> && range_constexpr_size<Range>() != std::dynamic_extent;

template <class F, class ... Args>
using function_return_type = decltype(std::invoke(std::declval<F>(), std::declval<Args>()...));

__RAZE_ALGORITHM_NAMESPACE_END
