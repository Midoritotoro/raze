#pragma once

#include <raze/algorithm/find/Equal.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct ends_with_t : Traits {
    template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
        std::input_iterator It2, std::sentinel_for<It2> Sent2,
        class Predicate = std::equal_to<>, class Proj1 = std::identity, class Proj2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(It1 first1, Sent1 sent1,
        It2 first2, Sent2 sent2, Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
            requires (std::forward_iterator<It1> || std::sized_sentinel_for<Sent1, It1>) &&
                (std::forward_iterator<It2> || std::sized_sentinel_for<Sent2, It2>) &&
                std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>
    {
        const auto size1 = std::ranges::distance(first1, sent1);
        const auto size2 = std::ranges::distance(first2, sent2);

        if (size1 < size2) return false;
        std::ranges::advance(first1, size1 - size2);

        return algorithm::equal[Traits::traits()](std::move(first1), std::move(sent1),
            std::move(first2), std::move(sent2), traits::fwd_fn(pred), traits::fwd_fn(proj1), 
            traits::fwd_fn(proj2));
    }

    template <std::ranges::input_range R1, std::ranges::input_range R2,
        class Predicate = std::equal_to<>, class Proj1 = std::identity, class Proj2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R1&& r1, R2&& r2, Predicate pred = {},
        Proj1 proj1 = {}, Proj2 proj2 = {}) const requires (std::ranges::forward_range<R1> || std::ranges::sized_range<R1>) &&
            (std::ranges::forward_range<R2> || std::ranges::sized_range<R2>) && std::indirectly_comparable<
                std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>, Predicate, Proj1, Proj2>
    {
        return (*this)(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), 
            std::ranges::end(r2), traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
    }
};

constexpr inline auto ends_with = options::function_with_traits<ends_with_t>;

__RAZE_ALGORITHM_NAMESPACE_END