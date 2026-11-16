#pragma once

#include <raze/algorithm/find/FindLastIf.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_last_t : Traits {
    template <std::forward_iterator It, std::sentinel_for<It> Sent, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent last, const T& value, Projection proj = {}) const
            requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
    {
        return find_last_if[Traits::traits()](std::move(first), std::move(last), algorithm::equal_to(
            function_return_type<Projection, std::iter_value_t<It>>(value)), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, const T& value, Projection proj = {}) const
            requires(std::indirect_binary_predicate<std::ranges::equal_to, 
                std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
    {
        return find_last_if[Traits::traits()](std::forward<R>(r), algorithm::equal_to(
            function_return_type<Projection, std::ranges::range_value_t<R>>(value)), traits::fwd_fn(proj));
    }
};

constexpr inline auto find_last = options::function_with_traits<find_last_t>;

__RAZE_ALGORITHM_NAMESPACE_END