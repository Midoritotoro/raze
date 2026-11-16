#pragma once

#include <raze/algorithm/find/FindLastIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_last_if_not_t : Traits {
    template <std::forward_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return find_last_if[Traits::traits()](std::move(first), std::move(last), make_not_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return find_last_if[Traits::traits()](std::forward<R>(r), make_not_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto find_last_if_not = options::function_with_traits<find_last_if_not_t>;

__RAZE_ALGORITHM_NAMESPACE_END