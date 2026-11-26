#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct all_of_t : Traits, dispatchable<all_of_t<Traits>> {
    template <source Source, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection _proj;
        bool _result = true;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type> && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_unary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        constexpr explicit kernel(Source&& source, Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()), _sentinel(_source.uend()),
              _predicate(std::move(pred)), _proj(std::move(proj))
        {}

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            for (; _iterator != _sentinel; ++_iterator) {
                if (!std::invoke(_predicate, std::invoke(_proj, *_iterator))) {
                    _result = false;
                    return;
                }
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

            raze_disable_unrolling do {
                if (!vx::all_of(_predicate(_proj(vx::load<Tag>(ptr))))) {
                    _result = false;
                    return false;
                }

                advance_bytes(ptr, sizeof(Tag));
            } while (ptr != aligned_end);

            source_type::from_ptr(_iterator, ptr);
            return true;
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            if (ignore.tail_bytes() == 0) return;

            const auto mask = _predicate(_proj(vx::load<Tag>[ignore](std::to_address(_iterator))));
            _result = vx::all_of[ignore](mask);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }

        raze_nodiscard constexpr raze_always_inline bool result() const {
            return _result;
        }
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R&& r, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_query
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks if a unary predicate returns `true` for all elements in a range.
 *
 * 1) Evaluates `bool(std::invoke(pred, std::invoke(proj, *i)))` for elements in `[first, last)` and returns
 *    `true` if the predicate holds for every element (or if the range is empty).
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `all_of(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
 * constexpr bool all_of( It first, Sent last, Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr bool all_of( R&& r, Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `pred`          - unary predicate to apply to each projected element
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * `true` if `pred` holds for every projected element in the range (or if the range is empty), `false` otherwise.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `N` applications of `pred` and `proj` (short-circuiting on the first falsifying element or SIMD chunk).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD short-circuits on the first failing vector chunk via `raze::vx::all_of`
 *   and evaluates remaining tail elements using masked `raze::vx::all_of`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/AllOf.h>
 *
 * int main() {
 *     const std::vector<int> v{2, 4, 6, 8, 10};
 *
 *     const bool all_even = raze::algorithm::all_of(v, [](auto x) { return (x & 1) == 0; });
 *     const bool all_gt5  = raze::algorithm::all_of(v, [](auto x) { return x > 5; });
 *
 *     std::cout << std::boolalpha << all_even << ' ' << all_gt5 << '\n';
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
 * - `raze::algorithm::any_of`
 * - `raze::algorithm::none_of`
 * - `raze::algorithm::find_if_not`
*/
constexpr inline auto all_of = options::function_with_traits<all_of_t>;

__RAZE_ALGORITHM_NAMESPACE_END