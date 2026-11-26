#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/memory/Memcpy.h>
#include <src/raze/algorithm/memory/Memmove.h>
#include <src/raze/algorithm/memory/Swap3Ranges.h>
#include <raze/algorithm/order/Reverse.h>
#include <raze/options/Options.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct rotate_t : Traits, dispatchable<rotate_t<Traits>> {
    template <source Source>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;

        using iterator_type = typename source_type::iterator_type;
        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using value_type = std::iter_value_t<unchecked_iterator_type>;
        using vector_value_type = void;

        static consteval bool vectorizable() noexcept {
            return false;
        }

        static consteval bool native() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && std::is_trivially_copyable_v<value_type>
                && traits::trivially_swappable<value_type>;
        }

        source_type _source;
        unchecked_iterator_type _first;
        unchecked_iterator_type _middle;
        unchecked_sentinel_type _sent;
        unchecked_iterator_type _last;

        constexpr explicit kernel(Source&& source, iterator_type middle)
            : _source(std::forward<Source>(source)),
              _first(_source.ubegin()),
              _middle(source_type::unwrap(std::move(middle))),
              _sent(_source.uend()),
              _last(_first)
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _first == _middle || _middle == _sent;
        }

        constexpr raze_always_inline std::ranges::subrange<iterator_type> default_result() const {
            if (_first == _middle) {
                auto last = _source.wrap(std::ranges::next(_first, _sent));
                return { last, last };
            }
            return { _source.wrap(_first), _source.wrap(_middle) };
        }

        raze_always_inline constexpr void operator()() {
            if constexpr (std::bidirectional_iterator<unchecked_iterator_type>) {
                reverse[options::fscalar](_first, _middle);
                _last = std::ranges::next(_middle, _sent);
                reverse[options::fscalar](_middle, _last);

                if constexpr (std::random_access_iterator<unchecked_iterator_type>) {
                    reverse[options::fscalar](_first, _last);
                    _first += _last - _middle;
                }
                else {
                    auto mid_last = _last;

                    raze_disable_unrolling
                    do {
                        std::ranges::iter_swap(_first, --mid_last);
                    } while (++_first != _middle && mid_last != _middle);

                    reverse[options::fscalar](_first, mid_last);

                    if (_first == _middle)
                        _first = std::move(mid_last);
                }
            }
            else {
                auto next = _middle;

                raze_disable_unrolling
                do {
                    std::ranges::iter_swap(_first, next++);
                    if (++_first == _middle) _middle = next;
                } while (next != _sent);

                auto new_first = _first;

                while (_middle != _sent) {
                    next = _middle;

                    raze_disable_unrolling
                    do {
                        std::ranges::iter_swap(_first, next++);
                        if (++_first == _middle) _middle = next;
                    } while (next != _sent);
                }

                _first = std::move(new_first);
                _last = std::move(_middle);
            }
        }

        raze_always_inline void operator()(native_tag) requires(native()) {
            const auto [orig_first, orig_last] = source_type::to_raw_range(_first, _sent);
            auto* middle_ptr = std::to_address(_middle);

            auto* first_ptr = orig_first;
            auto* last_ptr = orig_last;
            const auto right_total = bytes_distance(middle_ptr, last_ptr);

            alignas(64) u8 buf[512];

            auto swap_blocks = [&buf] (void* a, void* b, sizetype size) raze_always_inline_lambda {
                auto* pa = static_cast<u8*>(a);
                auto* pb = static_cast<u8*>(b);

                while (size != 0) {
                    const auto chunk = (size < sizeof(buf)) ? size : sizeof(buf);
                    __raze_memcpy(buf, pa, chunk);
                    __raze_memcpy(pa, pb, chunk);
                    __raze_memcpy(pb, buf, chunk);
                    pa += chunk;
                    pb += chunk;
                    size -= chunk;
                }
            };

            for (;;) {
                const auto left_size = bytes_distance(first_ptr, middle_ptr);
                const auto right_size = bytes_distance(middle_ptr, last_ptr);

                if (left_size <= right_size) {
                    if (left_size == 0) break;

                    if (left_size <= sizeof(buf) && (left_size <= 128 || right_size >= left_size * 2)) {
                        __raze_memcpy(buf, first_ptr, left_size);
                        __raze_memmove(first_ptr, middle_ptr, right_size);
                        __raze_memcpy(bytes_pointer_offset(first_ptr, right_size), buf, left_size);
                        break;
                    }

                    auto* mid2 = last_ptr;
                    rewind_bytes(mid2, left_size);

                    if (left_size * 2 > right_size) {
                        swap_blocks(first_ptr, mid2, left_size);
                        last_ptr = mid2;
                    }
                    else {
                        auto* mid3 = mid2;
                        rewind_bytes(mid3, left_size);
                        __swap_3_ranges[options::drop_alignment<Traits>()](mid2, last_ptr, first_ptr, mid3);
                        last_ptr = mid3;
                    }
                }
                else {
                    if (right_size == 0) break;

                    if (right_size <= sizeof(buf) && (right_size <= 128 || left_size >= right_size * 2)) {
                        __raze_memcpy(buf, middle_ptr, right_size);
                        __raze_memmove(bytes_pointer_offset(first_ptr, right_size), first_ptr, left_size);
                        __raze_memcpy(first_ptr, buf, right_size);
                        break;
                    }

                    if (right_size * 2 > left_size) {
                        swap_blocks(first_ptr, middle_ptr, right_size);
                        advance_bytes(first_ptr, right_size);
                    }
                    else {
                        __swap_3_ranges[options::drop_alignment<Traits>()](middle_ptr, last_ptr, bytes_pointer_offset(first_ptr, right_size), first_ptr);
                        advance_bytes(first_ptr, right_size * 2);
                    }
                }
            }

            source_type::from_ptr(_first, bytes_pointer_offset(orig_first, right_total));
            source_type::from_ptr(_last, orig_last);
        }

        constexpr raze_always_inline std::ranges::subrange<iterator_type> result() const {
            return { _source.wrap(_first), _source.wrap(_last) };
        }
    };

    template <std::permutable It, std::sentinel_for<It> Sent>
    constexpr raze_always_inline std::ranges::subrange<It> operator()(It first, It middle, Sent last) const {
        return this->dispatch(get_source(std::move(first), std::move(last)), std::move(middle));
    }

    template <std::ranges::forward_range R>
    constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
    operator()(R&& r, std::ranges::iterator_t<R> middle) const
        requires(std::permutable<std::ranges::iterator_t<R>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), std::move(middle));
    }
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Performs a left rotation on a range of elements in place.
 *
 * 1) Swaps the elements in the range `[first, last)` in such a way that the element `*middle`
 *    becomes the first element of the new range and `*(middle - 1)` becomes the last element.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `rotate(std::ranges::begin(r), middle, std::ranges::end(r))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable It, std::sentinel_for<It> Sent >
 * constexpr std::ranges::subrange<It>
 * rotate( It first, It middle, Sent last );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R >
 *   requires(std::permutable<std::ranges::iterator_t<R>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * rotate( R&& r, std::ranges::iterator_t<R> middle );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to rotate
 * - `r`             - the range of elements to rotate
 * - `middle`        - iterator to the element that should appear at the beginning of the rotated range
 *
 * ### Return value
 * A `std::ranges::subrange` `{new_first, end}` where `new_first` points to the new location of the
 * original first element (`first + (last - middle)`) and `end` compares equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear in `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Contiguous trivially copyable ranges take the native fast path using stack-buffered `memcpy`/`memmove`
 *   and SIMD-accelerated 3-range block swaps.
 * - Scalar fallback uses three-reversal for bidirectional iterators and forward cycle swapping for
 *   forward-only iterators.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/order/Rotate.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5, 6};
 *
 *     auto tail = raze::algorithm::rotate(v, v.begin() + 2);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << "| old first at index " << (tail.begin() - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 3 4 5 6 1 2 | old first at index 4
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::rotate_copy`
 * - `raze::algorithm::reverse`
*/
constexpr inline auto rotate = options::function_with_traits<rotate_t>;

__RAZE_ALGORITHM_NAMESPACE_END