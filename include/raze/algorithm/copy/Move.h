#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct move_t : Traits, dispatchable<move_t<Traits>> {
	template <source Source, destination Destination>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using out_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address destination_type _dest;
		unchecked_iterator_type _in;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_out_iterator_type _out;

		constexpr explicit kernel(Source&& src, Destination&& dst)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin())
		{}

		raze_always_inline constexpr void operator()() {
			for (; _in != _sent; ++_in, ++_out)
				*_out = std::ranges::iter_move(_in);
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::move_result<iterator_type, out_iterator_type>
		result() {
			return { _source.wrap(std::move(_in)), _dest.wrap(std::move(_out)) };
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out>
	constexpr raze_always_inline std::ranges::move_result<It, Out>
	operator()(It first, Sent last, Out result) const
		requires(std::indirectly_movable<It, Out>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(result)));
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out>
	constexpr raze_always_inline std::ranges::move_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result) const
		requires(std::indirectly_movable<std::ranges::iterator_t<R>, Out>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(result)));
	}
};

/**
 * @ingroup raze_algo_copy
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Moves elements from a range to another range in forward order.
 *
 * 1) Moves elements from `[first, last)` into the destination range beginning at `result` in order,
 *    assigning `std::ranges::iter_move(i)` for each element.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `move(std::ranges::begin(r), std::ranges::end(r), std::move(result))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out >
 *   requires(std::indirectly_movable<It, Out>)
 * constexpr std::ranges::move_result<It, Out>
 * move( It first, Sent last, Out result );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable Out >
 *   requires(std::indirectly_movable<std::ranges::iterator_t<R>, Out>)
 * constexpr std::ranges::move_result<std::ranges::borrowed_iterator_t<R>, Out>
 * move( R&& r, Out result );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the source range of elements to move
 * - `r`             - the source range of elements to move
 * - `result`        - the beginning of the destination range
 *
 * ### Return value
 * A `std::ranges::move_result` containing:
 * - `in`: an iterator comparing equal to `last` (or `std::ranges::end(r)`).
 * - `out`: an iterator pointing past the last moved element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`) move assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - The behavior is undefined if `result` is within `[first, last)`. Use `raze::algorithm::move_backward`
 *   for overlapping ranges shifting to the right.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <memory>
 * #include <vector>
 * #include <raze/algorithm/copy/Move.h>
 *
 * int main() {
 *     std::vector<std::unique_ptr<int>> src;
 *     src.push_back(std::make_unique<int>(10));
 *     src.push_back(std::make_unique<int>(20));
 *
 *     std::vector<std::unique_ptr<int>> dst(src.size());
 *     raze::algorithm::move(src, dst.begin());
 *     std::cout << *dst[0] << ' ' << *dst[1] << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::move_backward`
 * - `raze::algorithm::copy`
 */
constexpr inline auto move = options::function_with_traits<move_t>;

__RAZE_ALGORITHM_NAMESPACE_END

