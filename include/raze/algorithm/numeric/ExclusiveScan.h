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
struct exclusive_scan_t : Traits, dispatchable<exclusive_scan_t<Traits>> {
	template <source Source, destination Destination, class T, class BinaryOp>
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
		T _init;
		raze_no_unique_address BinaryOp _op;

		constexpr explicit kernel(Source&& src, Destination&& dst, T init, BinaryOp op)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _init(std::move(init)),
			  _op(std::move(op))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _in != _sent; ++_in, ++_out) {
				auto next = std::invoke(_op, _init, *_in);
				*_out = std::move(_init);
				_init = std::move(next);
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::in_out_result<iterator_type, out_iterator_type> result() {
			return { _source.wrap(std::move(_in)), _dest.wrap(std::move(_out)) };
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
		class T = std::iter_value_t<It>, class BinaryOp = std::plus<>>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, T init, BinaryOp op = {}) const
		requires(std::indirectly_writable<Out, T> &&
			std::invocable<BinaryOp&, const T&, std::iter_reference_t<It>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(result)), std::move(init), traits::fwd_fn(op));
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class T = std::ranges::range_value_t<R>, class BinaryOp = std::plus<>>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, T init, BinaryOp op = {}) const
		requires(std::indirectly_writable<Out, T> &&
			std::invocable<BinaryOp&, const T&, std::ranges::range_reference_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(result)), std::move(init), traits::fwd_fn(op));
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Computes an exclusive prefix scan over a range starting from `init`.
 *
 * 1) Writes to `result[i]` the accumulation of `init` and elements `[0, i)` of `[first, last)` under `op`
 *    (excluding element `i` itself). Safe when `result == first` (in-place).
 *
 * 2) Same as (1), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out,
 *           class T = std::iter_value_t<It>, class BinaryOp = std::plus<> >
 * constexpr std::ranges::in_out_result<It, Out>
 * exclusive_scan( It first, Sent last, Out result, T init, BinaryOp op = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable Out,
 *           class T = std::ranges::range_value_t<R>, class BinaryOp = std::plus<> >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * exclusive_scan( R&& r, Out result, T init, BinaryOp op = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - input range of elements
 * - `result`             - beginning of destination range (may equal `first`)
 * - `init`               - initial value written at index 0
 * - `op`                 - associative binary operation (defaults to `std::plus<>`)
 *
 * ### Return value
 * A `std::ranges::in_out_result` `{in, out}`.
 *
 * ### Complexity
 * Linear: `O(N)` applications of `op`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/numeric/ExclusiveScan.h>
 *
 * int main() {
 *     std::vector<int> v{3, 1, 4, 1, 5};
 *     raze::algorithm::exclusive_scan(v, v.begin(), 0);
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
 * - `raze::algorithm::transform_exclusive_scan`
 */
constexpr inline auto exclusive_scan = options::function_with_traits<exclusive_scan_t>;

__RAZE_ALGORITHM_NAMESPACE_END

