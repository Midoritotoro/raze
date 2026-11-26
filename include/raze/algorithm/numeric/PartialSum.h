#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct partial_sum_t : Traits, dispatchable<partial_sum_t<Traits>> {
	template <source Source, destination Destination, class BinaryOp>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using dest_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using out_iterator_type = typename dest_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename dest_type::unchecked_iterator_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address dest_type _dest;
		unchecked_iterator_type _in;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_out_iterator_type _out;
		raze_no_unique_address BinaryOp _op;

		constexpr explicit kernel(Source&& src, Destination&& dst, BinaryOp op)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _op(std::move(op))
		{}

		raze_always_inline constexpr void operator()() {
			if (_in == _sent) return;

			std::iter_value_t<unchecked_iterator_type> acc = *_in;
			*_out = acc;
			++_out;
			for (++_in; _in != _sent; ++_in, ++_out) {
				acc = std::invoke(_op, std::move(acc), *_in);
				*_out = acc;
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::in_out_result<iterator_type, out_iterator_type> result() {
			return { _source.wrap(std::move(_in)), _dest.wrap(std::move(_out)) };
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
		class BinaryOp = std::plus<>>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, BinaryOp op = {}) const
		requires(std::indirectly_writable<Out, const std::iter_value_t<It>&> &&
			std::invocable<BinaryOp&, std::iter_value_t<It>, std::iter_reference_t<It>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(result)), traits::fwd_fn(op));
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class BinaryOp = std::plus<>>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, BinaryOp op = {}) const
		requires(std::indirectly_writable<Out, const std::ranges::range_value_t<R>&> &&
			std::invocable<BinaryOp&, std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(result)), traits::fwd_fn(op));
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Computes sequential partial sums (inclusive prefix scan) of a range.
 *
 * 1) Writes sequential running totals of `[first, last)` under `op` into the destination range
 *    beginning at `result`. Safe when `result == first` (in-place).
 *
 * 2) Same as (1), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class BinaryOp = std::plus<> >
 * constexpr std::ranges::in_out_result<It, Out>
 * partial_sum( It first, Sent last, Out result, BinaryOp op = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           std::weakly_incrementable Out, class BinaryOp = std::plus<> >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * partial_sum( R&& r, Out result, BinaryOp op = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - input range of elements
 * - `r`             - input range of elements
 * - `result`        - beginning of destination range (may equal `first`)
 * - `op`            - binary operation (defaults to `std::plus<>`)
 *
 * ### Return value
 * A `std::ranges::in_out_result` `{in, out}`.
 *
 * ### Complexity
 * Linear: `max(N - 1, 0)` applications of `op` and `N` assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/numeric/PartialSum.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4};
 *     raze::algorithm::partial_sum(v, v.begin());
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::inclusive_scan`
 * - `raze::algorithm::exclusive_scan`
 * - `raze::algorithm::adjacent_difference`
 */
constexpr inline auto partial_sum = options::function_with_traits<partial_sum_t>;

__RAZE_ALGORITHM_NAMESPACE_END

