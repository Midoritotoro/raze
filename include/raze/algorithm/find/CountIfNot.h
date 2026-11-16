#pragma once

#include <raze/algorithm/find/CountIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct count_if_not_t : Traits {
    template <std::input_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::iter_difference_t<It>
        operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return count_if[Traits::traits()](std::move(first), std::move(last),
            make_not_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<R>
        operator()(R&& r, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return count_if[Traits::traits()](std::forward<R>(r), make_not_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto count_if_not = options::function_with_traits<count_if_not_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
