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
        const auto size1 = iter_distance(first1, sent1);
        const auto size2 = iter_distance(first2, sent2);

        if (size1 < size2) return false;
        const auto diff = static_cast<std::iter_difference_t<It1>>(size1 - size2);
        if (diff > 0) std::ranges::advance(first1, diff, sent1);

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
        const auto size1 = std::ranges::distance(r1);
        const auto size2 = std::ranges::distance(r2);

        if (size1 < size2) return false;
        auto first1 = std::ranges::begin(r1);
        auto sent1 = std::ranges::end(r1);
        const auto diff = static_cast<std::ranges::range_difference_t<R1>>(size1 - size2);
        if (diff > 0) std::ranges::advance(first1, diff, sent1);

        return algorithm::equal[Traits::traits()](std::move(first1), std::move(sent1),
            std::ranges::begin(r2), std::ranges::end(r2),
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
    }
};

/**
 * @ingroup raze_algo_query
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether the end of a range matches another range.
 *
 * 1) Returns `true` if `[first1, sent1)` has at least as many elements as `[first2, sent2)` and its suffix
 *    matches `[first2, sent2)` element-wise under `pred`, `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `ends_with(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), pred, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::input_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires (std::forward_iterator<It1> || std::sized_sentinel_for<Sent1, It1>) &&
 *            (std::forward_iterator<It2> || std::sized_sentinel_for<Sent2, It2>) &&
 *            std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>
 * constexpr bool ends_with( It1 first1, Sent1 sent1, It2 first2, Sent2 sent2,
 *                           Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires (std::ranges::forward_range<R1> || std::ranges::sized_range<R1>) &&
 *            (std::ranges::forward_range<R2> || std::ranges::sized_range<R2>) &&
 *            std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                                       Predicate, Proj1, Proj2>
 * constexpr bool ends_with( R1&& r1, R2&& r2,
 *                           Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `sent1` - the range to test
 * - `first2`, `sent2` - the suffix range to match
 * - `r1`              - the range to test
 * - `r2`              - the suffix range to match
 * - `pred`            - binary predicate applied to projected elements (defaults to `std::equal_to<>`)
 * - `proj1`, `proj2`  - projections to apply to elements of the first and second ranges (default to `std::identity`)
 *
 * ### Return value
 * `true` if the first range ends with the second range (always `true` when the second range is empty),
 * `false` otherwise.
 *
 * ### Complexity
 * Linear in `std::ranges::distance(first2, sent2)` for comparisons (plus distance computation / advancement
 * when iterators are not random-access). Returns `false` without element comparisons if `size1 < size2`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::equal`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Returns `false` without element comparisons when `size1 < size2`; otherwise advances `first1`
 *   by `size1 - size2` and delegates suffix comparison to `raze::algorithm::equal`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/EndsWith.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5};
 *     const std::vector<int> suffix{3, 4, 5};
 *     const std::vector<int> other{2, 3, 4};
 *
 *     std::cout << std::boolalpha
 *               << raze::algorithm::ends_with(v, suffix) << ' '
 *               << raze::algorithm::ends_with(v, other) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * true false
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::starts_with`
 * - `raze::algorithm::equal`
 * - `raze::algorithm::mismatch`
 * - `raze::algorithm::find_end`
*/
constexpr inline auto ends_with = options::function_with_traits<ends_with_t>;

__RAZE_ALGORITHM_NAMESPACE_END