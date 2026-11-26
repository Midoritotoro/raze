#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/math/Math.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_if_t : Traits, dispatchable<find_if_t<Traits>> {
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

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type> && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_unary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        constexpr explicit kernel(Source&& source, Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _predicate(std::move(pred)),
              _proj(std::move(proj))
        {}

        raze_always_inline constexpr void operator()(autovectorizable_tag)
            requires(contiguous_source<source_type> && !traits::is_iterator_volatile_v<iterator_type>)
        {
            auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);

            for (; first != last; ++first)
                if (std::invoke(_predicate, std::invoke(_proj, *first)))
                    break;

            source_type::from_ptr(_iterator, first);
        }

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            for (; _iterator != _sentinel; ++_iterator) {
                if (std::invoke(_predicate, std::invoke(_proj, *_iterator)))
                    break;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

            raze_disable_unrolling do {
                const auto mask = _predicate(_proj(vx::load<Tag>(ptr)));

                if (vx::any_of(mask)) {
                    source_type::from_ptr(_iterator, ptr + vx::find_first_set[vx::not_null](mask));
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

            auto* ptr = std::to_address(_iterator);
            const auto mask = _predicate(_proj(vx::load<Tag>[ignore](ptr)));

            const auto offset = math::min(static_cast<sizetype>(vx::find_first_set[ignore](mask)), ignore.tail_elements());
            source_type::from_ptr(_iterator, ptr + offset);
        }

        constexpr raze_always_inline iterator_type result() {
            return _source.wrap(std::move(_iterator));
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent,
        class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline It
    operator()(It first, Sent sent, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(sent)), 
            traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
    operator()(R&& r, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo_find
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the first element in a range satisfying a unary predicate.
 *
 * 1) Searches `[first, sent)` for the first iterator `i` such that
 *    `bool(std::invoke(pred, std::invoke(proj, *i)))` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `find_if(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
 * constexpr It find_if( It first, Sent sent, Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * find_if( R&& r, Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to search
 * - `r`             - the range of elements to search
 * - `pred`          - unary predicate returning `true` for the target element
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator to the first element satisfying `pred`, or an iterator comparing equal to
 * `sent` (or `std::ranges::end(r)`) if no such element is found.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, sent)` (or `std::ranges::distance(r)`):
 * at most `N` applications of `pred` and `proj` (short-circuiting on the first match).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Auto-vectorization:** Provides an `autovec` kernel for contiguous ranges (see @ref autovec_behavior).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD short-circuits on the first matching vector chunk via `raze::vx::any_of` +
 *   `raze::vx::find_first_set`, with masked tail evaluation.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/FindIf.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 3, 5, 8, 9, 10};
 *
 *     if (auto it = raze::algorithm::find_if(v, [](auto x) { return (x & 1) == 0; }); it != v.end())
 *         std::cout << "First even element: " << *it << " at index " << (it - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * First even element: 8 at index 3
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find`
 * - `raze::algorithm::find_if_not`
 * - `raze::algorithm::find_last_if`
 * - `raze::algorithm::any_of`
*/
constexpr inline auto find_if = options::function_with_traits<find_if_t>;

__RAZE_ALGORITHM_NAMESPACE_END