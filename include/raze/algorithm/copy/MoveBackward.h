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
struct move_backward_t : Traits, dispatchable<move_backward_t<Traits>> {
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
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _last;
		unchecked_out_iterator_type _out;

		constexpr explicit kernel(Source&& src, Destination&& dst)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_first, _sent)),
			  _out(_dest.ubegin())
		{}

		raze_always_inline constexpr void operator()() {
			auto cur = _last;
			while (cur != _first) {
				--cur;
				--_out;
				*_out = std::ranges::iter_move(cur);
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::move_backward_result<iterator_type, out_iterator_type> result() {
			return { _source.wrap(std::move(_last)), _dest.wrap(std::move(_out)) };
		}
	};

	template <std::bidirectional_iterator It, std::sentinel_for<It> Sent, std::bidirectional_iterator Out>
	constexpr raze_always_inline std::ranges::move_backward_result<It, Out>
	operator()(It first, Sent last, Out result) const
		requires(std::indirectly_movable<It, Out>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(result)));
	}

	template <std::ranges::bidirectional_range R, std::bidirectional_iterator Out>
	constexpr raze_always_inline std::ranges::move_backward_result<std::ranges::borrowed_iterator_t<R>, Out>
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
 * @brief Moves elements from a range to another range in backward order.
 *
 * 1) Moves elements from `[first, last)` into the range ending at `result`, starting from the last
 *    element and proceeding to the first, preserving the relative order of elements.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `move_backward(std::ranges::begin(r), std::ranges::end(r), std::move(result))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent,
 *           std::bidirectional_iterator Out >
 *   requires(std::indirectly_movable<It, Out>)
 * constexpr std::ranges::move_backward_result<It, Out>
 * move_backward( It first, Sent last, Out result );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R, std::bidirectional_iterator Out >
 *   requires(std::indirectly_movable<std::ranges::iterator_t<R>, Out>)
 * constexpr std::ranges::move_backward_result<std::ranges::borrowed_iterator_t<R>, Out>
 * move_backward( R&& r, Out result );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the source range of elements to move
 * - `r`             - the source range of elements to move
 * - `result`        - the end of the destination range
 *
 * ### Return value
 * A `std::ranges::move_backward_result` containing:
 * - `in`: an iterator comparing equal to `last` (or `std::ranges::end(r)`).
 * - `out`: an iterator to the first moved element in the destination range (`result - N`).
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`) move assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - The behavior is undefined if `result` is within `(first, last]`. Use `raze::algorithm::move`
 *   for overlapping ranges shifting to the left.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <string>
 * #include <vector>
 * #include <raze/algorithm/copy/MoveBackward.h>
 *
 * int main() {
 *     std::vector<std::string> v{"a", "b", "c", "", ""};
 *     raze::algorithm::move_backward(v.begin(), v.begin() + 3, v.end());
 *     for (const auto& s : v) std::cout << '[' << s << "] ";
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::move`
 * - `raze::algorithm::copy_backward`
 */
constexpr inline auto move_backward = options::function_with_traits<move_backward_t>;

__RAZE_ALGORITHM_NAMESPACE_END

