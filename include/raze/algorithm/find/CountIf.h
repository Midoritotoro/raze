#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct count_if_t : Traits, dispatchable<count_if_t<Traits>> {
    template <source Source, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;
        using difference_type = std::iter_difference_t<iterator_type>;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        raze_no_unique_address source_type _source;
        unchecked_iterator_type _iterator;
        raze_no_unique_address unchecked_sentinel_type _sentinel;
        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection _proj;
        difference_type _count = 0;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_unary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        constexpr explicit kernel(Source&& src, Predicate pred, Projection proj)
            : _source(std::forward<Source>(src)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _predicate(std::move(pred)),
              _proj(std::move(proj))
        {}

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            for (; _iterator != _sentinel; ++_iterator)
                _count += static_cast<difference_type>(static_cast<bool>(std::invoke(_predicate, std::invoke(_proj, *_iterator))));
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            vx::counter<Tag> counter;

            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

            if constexpr (!vx::counter<Tag>::needs_chunking()) {
                raze_disable_unrolling do {
                    counter.count(_predicate(_proj(vx::load<Tag>(ptr))));
                    advance_bytes(ptr, sizeof(Tag));
                } while (ptr != aligned_end);

                _count += counter.result();
            }
            else {
                constexpr sizetype max_portion_bytes = sizeof(typename vx::counter<Tag>::index_type) < sizeof(sizetype)
                    ? static_cast<sizetype>(vx::counter<Tag>::portion_size()) * sizeof(Tag)
                    : math::max_limit<sizetype>();

                auto remaining = aligned_size;

                raze_disable_unrolling while (remaining > 0) {
                    const auto portion_bytes = math::min(remaining, max_portion_bytes);
                    remaining -= portion_bytes;
                    const auto chunk_end = bytes_pointer_offset(ptr, portion_bytes);

                    raze_disable_unrolling do {
                        counter.count(_predicate(_proj(vx::load<Tag>(ptr))));
                        advance_bytes(ptr, sizeof(Tag));
                    } while (ptr != chunk_end);

                    _count += counter.result();
                    counter.reset();
                }
            }

            source_type::from_ptr(_iterator, ptr);
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            if (ignore.tail_bytes() == 0) return;

            _count += vx::count_set[ignore](_predicate(_proj(vx::load<Tag>[ignore](std::to_address(_iterator)))));
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }

        constexpr raze_always_inline difference_type result() const {
            return _count;
        }
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent,
        class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::iter_difference_t<It>
    operator()(It first, Sent sent, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(sent)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<R>
    operator()(R&& r, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_count
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Counts the elements in a range that satisfy a unary predicate.
 *
 * 1) Counts all elements in `[first, sent)` for which `bool(std::invoke(pred, std::invoke(proj, *i)))`
 *    evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `count_if(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
 * constexpr std::iter_difference_t<It>
 * count_if( It first, Sent sent, Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::ranges::range_difference_t<R>
 * count_if( R&& r, Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `pred`          - unary predicate returning `true` for elements to count
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * The number of elements satisfying `pred`.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, sent)` (or `std::ranges::distance(r)`)
 * applications of `pred` and `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`.
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD accumulates matches via `raze::vx::counter` (automatically chunking narrow lane
 *   counters to prevent overflow) and counts remaining tail elements with masked `raze::vx::count_set`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/CountIf.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
 *
 *     const auto evens = raze::algorithm::count_if(v, [](auto x) { return (x & 1) == 0; });
 *     std::cout << "Even count: " << evens << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Even count: 4
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::count`
 * - `raze::algorithm::count_if_not`
 * - `raze::algorithm::any_of`
 * - `raze::algorithm::find_if`
*/
constexpr inline auto count_if = options::function_with_traits<count_if_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
