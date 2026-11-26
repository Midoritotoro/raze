#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/memory/Memcpy.h>
#include <raze/options/Options.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct rotate_copy_t : Traits, dispatchable<rotate_copy_t<Traits>> {
    template <source Source, destination Destination>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using destination_type = std::remove_cvref_t<Destination>;

        using iterator_type = typename source_type::iterator_type;
        using out_iterator_type = typename destination_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
        using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;

        template <class It>
        static consteval auto deduce_value_type() noexcept {
            if constexpr (std::indirectly_readable<It>)
                return std::type_identity<std::iter_value_t<It>>{};
            else
                return std::type_identity<void>{};
        }

        using in_t = std::iter_value_t<unchecked_iterator_type>;
        using out_t = typename decltype(deduce_value_type<unchecked_out_iterator_type>())::type;
        using vector_value_type = void;

        static consteval bool vectorizable() noexcept {
            return false;
        }

        static consteval bool native() noexcept {
            return contiguous_source<source_type>
                && contiguous_destination<destination_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && !traits::is_iterator_volatile_v<out_iterator_type>
                && std::is_trivially_copyable_v<in_t>
                && std::is_trivially_copy_assignable_v<in_t>
                && std::same_as<in_t, out_t>;
        }

        source_type _source;
        destination_type _dest;
        unchecked_iterator_type _first;
        unchecked_iterator_type _middle;
        unchecked_sentinel_type _sent;
        unchecked_iterator_type _last;
        unchecked_out_iterator_type _out;

        constexpr explicit kernel(Source&& source, iterator_type middle, Destination&& dest)
            : _source(std::forward<Source>(source)),
              _dest(std::forward<Destination>(dest)),
              _first(_source.ubegin()),
              _middle(source_type::unwrap(std::move(middle))),
              _sent(_source.uend()),
              _last(_first),
              _out(_dest.ubegin())
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _first == _sent;
        }

        constexpr raze_always_inline std::ranges::in_out_result<iterator_type, out_iterator_type> default_result() const {
            return { _source.wrap(_first), _dest.wrap(_out) };
        }

        raze_always_inline constexpr void operator()() {
            _last = _middle;
            raze_disable_unrolling
            for (; _last != _sent; ++_last, ++_out)
                *_out = *_last;
            raze_disable_unrolling
            for (; _first != _middle; ++_first, ++_out)
                *_out = *_first;
        }

        raze_always_inline void operator()(native_tag) requires(native()) {
            const auto [first_ptr, last_ptr] = source_type::to_raw_range(_first, _sent);
            auto* middle_ptr = std::to_address(_middle);
            auto* out_ptr = std::to_address(_out);

            const auto right_size = bytes_distance(middle_ptr, last_ptr);
            const auto left_size = bytes_distance(first_ptr, middle_ptr);

            __raze_memcpy(out_ptr, middle_ptr, right_size);
            advance_bytes(out_ptr, right_size);
            __raze_memcpy(out_ptr, first_ptr, left_size);
            advance_bytes(out_ptr, left_size);

            source_type::from_ptr(_last, last_ptr);
            destination_type::from_ptr(_out, out_ptr);
        }

        constexpr raze_always_inline std::ranges::in_out_result<iterator_type, out_iterator_type> result() const {
            return { _source.wrap(_last), _dest.wrap(_out) };
        }
    };

    template <std::forward_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out>
    constexpr raze_always_inline std::ranges::in_out_result<It, Out>
        operator()(It first, It middle, Sent last, Out result) const requires(std::indirectly_copyable<It, Out>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)), std::move(middle), get_destination(std::move(result)));
    }

    template <std::ranges::forward_range R, std::weakly_incrementable Out>
    constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
        operator()(R&& r, std::ranges::iterator_t<R> middle, Out result) const
            requires(std::indirectly_copyable<std::ranges::iterator_t<R>, Out>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), std::move(middle), get_destination(std::move(result)));
    }
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies a range into another range while performing a left rotation.
 *
 * 1) Copies the elements from `[first, last)` to the range beginning at `result` such that
 *    `[middle, last)` is placed first, followed by `[first, middle)`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `rotate_copy(std::ranges::begin(r), middle, std::ranges::end(r), std::move(result))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out >
 *   requires(std::indirectly_copyable<It, Out>)
 * constexpr std::ranges::in_out_result<It, Out>
 * rotate_copy( It first, It middle, Sent last, Out result );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R, std::weakly_incrementable Out >
 *   requires(std::indirectly_copyable<std::ranges::iterator_t<R>, Out>)
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * rotate_copy( R&& r, std::ranges::iterator_t<R> middle, Out result );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the source range of elements to copy
 * - `r`             - the source range of elements to copy
 * - `middle`        - iterator to the element that becomes the first element of the copied range
 * - `result`        - the beginning of the destination range (must not overlap with the source range)
 *
 * ### Return value
 * A `std::ranges::in_out_result` containing:
 * - `in`: an iterator comparing equal to `last` (or `std::ranges::end(r)`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`) assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - The behavior is undefined if the source and destination ranges overlap.
 * - Contiguous ranges of identical trivially copyable types take the native fast path via two `memcpy` calls.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/order/RotateCopy.h>
 *
 * int main() {
 *     const std::vector<int> src{1, 2, 3, 4, 5, 6};
 *     std::vector<int> dst(src.size());
 *
 *     raze::algorithm::rotate_copy(src, src.begin() + 2, dst.begin());
 *     for (int x : dst) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 3 4 5 6 1 2 
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::rotate`
 * - `raze::algorithm::copy`
*/
constexpr inline auto rotate_copy = options::function_with_traits<rotate_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END