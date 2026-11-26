#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct reverse_t : Traits, dispatchable<reverse_t<Traits>> {
    template <source Source>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;

        using iterator_type = typename source_type::iterator_type;
        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;


        template <class T>
        static consteval auto deduce_vector_value_type() noexcept {
            if constexpr (sizeof(T) <= 8 && sizeof(T) != 0 && ((sizeof(T) & (sizeof(T) - 1)) == 0)) {
                if constexpr (vectorizable_value_type<T>) return T{};
                else return typename IntegerForSizeof<T>::Unsigned{};
            }
        }

        using in_t = std::iter_value_t<unchecked_iterator_type>;
        using vector_value_type = decltype(deduce_vector_value_type<in_t>());

        static consteval bool vectorizable() noexcept {
            return contiguous_source<Source> && traits::trivially_swappable<in_t>
                && !traits::is_iterator_volatile_v<iterator_type>
                && !std::same_as<vector_value_type, void>
                && sizeof(in_t) <= 8 && ((sizeof(in_t) & (sizeof(in_t) - 1)) == 0);
        }

        source_type _source;
        unchecked_iterator_type _front;
        unchecked_iterator_type _back;
        unchecked_iterator_type _end;

        constexpr explicit kernel(Source&& source)
            : _source(std::forward<Source>(source)),
              _front(_source.ubegin()),
              _back(std::ranges::next(_front, _source.uend())),
              _end(_back)
        {}

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            while (_front != _back) {
                --_back;
                if (_front == _back) break;
                std::ranges::iter_swap(_front, _back);
                ++_front;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            const auto pairs = aligned_size / sizeof(Tag) / 2;
            if (pairs == 0) return;

            auto* front = std::to_address(_front);
            auto* back = std::to_address(_back);

            raze_disable_unrolling
            for (sizetype i = 0; i < pairs; ++i) {
                rewind_bytes(back, sizeof(Tag));

                const auto vf = vx::load<Tag>(front);
                const auto vb = vx::load<Tag>(back);

                vx::store(front, vx::reverse(vb));
                vx::store(back, vx::reverse(vf));

                advance_bytes(front, sizeof(Tag));
            }

            source_type::from_ptr(_front, front);
            source_type::from_ptr(_back, back);
        }

        constexpr raze_always_inline iterator_type result() const {
            return _source.wrap(_end);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<Source>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }
    };

    template <std::bidirectional_iterator It, std::sentinel_for<It> Sent>
    constexpr raze_always_inline It operator()(It first, Sent last) const
        requires(std::permutable<It>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)));
    }

    template <std::ranges::bidirectional_range R>
    constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r) const
        requires(std::permutable<std::ranges::iterator_t<R>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)));
    }
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Reverses the order of elements in a range in place.
 *
 * 1) Reverses the order of the elements in the range `[first, last)`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `reverse(std::ranges::begin(r), std::ranges::end(r))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent >
 *   requires(std::permutable<It>)
 * constexpr It
 * reverse( It first, Sent last );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R >
 *   requires(std::permutable<std::ranges::iterator_t<R>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * reverse( R&& r );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to reverse
 * - `r`             - the range of elements to reverse
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * exactly `N / 2` swaps in the scalar path, or `O(N / (2 * W))` vector load-reverse-store pairs
 * in the explicit SIMD path.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD accelerates contiguous ranges of trivially swappable elements with power-of-two
 *   size `<= 8` bytes by reversing and swapping symmetric vector chunks from both ends via `raze::vx::reverse`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/order/Reverse.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5};
 *
 *     raze::algorithm::reverse(v);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 5 4 3 2 1 
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::reverse_copy`
 * - `raze::algorithm::rotate`
*/
constexpr inline auto reverse = options::function_with_traits<reverse_t>;

__RAZE_ALGORITHM_NAMESPACE_END