#pragma once

#include <raze/algorithm/find/AnyOf.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct contains_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(It first, Sent last, const T& value, Projection proj = {}) const
        requires(std::indirect_binary_predicate<std::ranges::equal_to, std::projected<It, Projection>, const T*>)
    {
        return any_of[Traits::traits()](std::move(first), std::move(last), equal_to(value), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class T, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R&& r, const T& value, 
        Projection proj = {}) const requires(std::indirect_binary_predicate<std::ranges::equal_to,
            std::projected<std::ranges::iterator_t<R>, Projection>, const T*>)
    {
        return any_of[Traits::traits()](std::forward<R>(r), equal_to(value), traits::fwd_fn(proj));
    }
};

constexpr inline auto contains = options::function_with_traits<contains_t>;

__RAZE_ALGORITHM_NAMESPACE_END