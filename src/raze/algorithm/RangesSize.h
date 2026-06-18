#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <ranges>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class>
constexpr inline auto __is_std_array_v = false;

template <class _Type_, sizetype _N_>
constexpr inline auto __is_std_array_v<std::array<_Type_, _N_>> = true;

template <class>
constexpr inline auto __is_std_span_v = false;

template <class _Type_, sizetype _N_>
inline constexpr bool __is_std_span_v<std::span<_Type_, _N_>> = true;

template <auto> 
struct __require_constant;

template <class _Range_>
constexpr auto __range_constexpr_size() noexcept {
    using _Raw_ = std::remove_cvref_t<_Range_>;

    if constexpr (std::is_bounded_array_v<_Raw_>) return std::extent_v<_Raw_>;
    else if constexpr (__is_std_array_v<_Raw_>) return std::tuple_size_v<_Raw_>;
    else if constexpr (__is_std_span_v<_Raw_>) return _Raw_::extent;
    else if constexpr (std::ranges::sized_range<_Raw_> && requires { typename __require_constant<_Raw_::size()>; }) return _Raw_::size();
    else return std::dynamic_extent;
}

template <class _Range_>
concept constexpr_sized_range = std::ranges::range<std::remove_cvref_t<_Range_>> && __range_constexpr_size<_Range_>() != std::dynamic_extent;

template <class _Func_, class ... _Args_>
using function_return_type = decltype(std::invoke(std::declval<_Func_>(), std::declval<_Args_>()...));

__RAZE_ALGORITHM_NAMESPACE_END
