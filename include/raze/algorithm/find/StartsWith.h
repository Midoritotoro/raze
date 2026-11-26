#pragma once

#include <raze/algorithm/find/Mismatch.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct starts_with_t : Traits {
    template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
        std::input_iterator It2, std::sentinel_for<It2> Sent2,
        class Predicate = std::equal_to<>, class Proj1 = std::identity, class Proj2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(
        It1 first1, Sent1 sent1, It2 first2, Sent2 sent2,
        Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
            requires std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>
    {
        if constexpr (std::sized_sentinel_for<Sent1, It1> && std::sized_sentinel_for<Sent2, It2>) {
            if (std::ranges::distance(first1, sent1) < std::ranges::distance(first2, sent2)) return false;
        }

        return algorithm::mismatch[Traits::traits()](std::move(first1), std::move(sent1),
            std::move(first2), sent2, traits::fwd_fn(pred), traits::fwd_fn(proj1), 
            traits::fwd_fn(proj2)).in2 == sent2;
    }

    template <std::ranges::input_range R1, std::ranges::input_range R2,
        class Predicate = std::equal_to<>, class Proj1 = std::identity, class Proj2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R1&& r1, R2&& r2, Predicate pred = {},
        Proj1 proj1 = {}, Proj2 proj2 = {}) const
            requires std::indirectly_comparable<std::ranges::iterator_t<R1>,
                std::ranges::iterator_t<R2>, Predicate, Proj1, Proj2>
    {
        if constexpr (std::ranges::sized_range<R1> && std::ranges::sized_range<R2>) {
            if (std::ranges::size(r1) < std::ranges::size(r2)) return false;
        }

        auto sent2 = std::ranges::end(r2);

        return algorithm::mismatch[Traits::traits()](std::forward<R1>(r1), std::forward<R2>(r2),
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2)).in2 == sent2;
    }
};

/**
 * @ingroup raze_algo_query
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether the beginning of a range matches another range.
 *
 * 1) Returns `true` if `[first1, sent1)` has at least as many elements as `[first2, sent2)` and its prefix
 *    matches `[first2, sent2)` element-wise under `pred`, `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `starts_with(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), pred, proj1, proj2)`.
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
 * constexpr bool starts_with( It1 first1, Sent1 sent1, It2 first2, Sent2 sent2,
 *                             Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires (std::ranges::forward_range<R1> || std::ranges::sized_range<R1>) &&
 *            (std::ranges::forward_range<R2> || std::ranges::sized_range<R2>) &&
 *            std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                                       Predicate, Proj1, Proj2>
 * constexpr bool starts_with( R1&& r1, R2&& r2,
 *                             Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `sent1` - the range to test
 * - `first2`, `sent2` - the prefix range to match
 * - `r1`              - the range to test
 * - `r2`              - the prefix range to match
 * - `pred`            - binary predicate applied to projected elements (defaults to `std::equal_to<>`)
 * - `proj1`, `proj2`  - projections to apply to elements of the first and second ranges (default to `std::identity`)
 *
 * ### Return value
 * `true` if the first range starts with the second range (always `true` when the second range is empty),
 * `false` otherwise.
 *
 * ### Complexity
 * - O(1) if both ranges/sentinels are sized and `len1 < len2`.
 * - Otherwise linear: at most `min(N1, N2)` applications of `pred`, `proj1`, and `proj2`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (forwarded to `raze::algorithm::mismatch`;
 * see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - For sized ranges/sentinels, returns `false` immediately when the first range is shorter than the prefix.
 * - Delegates element comparison to `raze::algorithm::mismatch`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/StartsWith.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5};
 *     const std::vector<int> prefix{1, 2, 3};
 *     const std::vector<int> other{2, 3, 4};
 *
 *     std::cout << std::boolalpha
 *               << raze::algorithm::starts_with(v, prefix) << ' '
 *               << raze::algorithm::starts_with(v, other) << '\n';
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
 * - `raze::algorithm::ends_with`
 * - `raze::algorithm::mismatch`
 * - `raze::algorithm::equal`
 * - `raze::algorithm::contains_subrange`
*/
constexpr inline auto starts_with = options::function_with_traits<starts_with_t>;

__RAZE_ALGORITHM_NAMESPACE_END