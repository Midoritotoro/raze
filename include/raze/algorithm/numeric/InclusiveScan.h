#pragma once

#include <raze/algorithm/numeric/PartialSum.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct inclusive_scan_t : Traits, dispatchable<inclusive_scan_t<Traits>> {
	template <source Source, destination Destination, class BinaryOp, class T>
	struct init_kernel {
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
		T _init;

		constexpr explicit init_kernel(Source&& src, Destination&& dst, BinaryOp op, T init)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _op(std::move(op)),
			  _init(std::move(init))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _in != _sent; ++_in, ++_out) {
				_init = std::invoke(_op, std::move(_init), *_in);
				*_out = _init;
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::in_out_result<iterator_type, out_iterator_type> result() {
			return { _source.wrap(std::move(_in)), _dest.wrap(std::move(_out)) };
		}
	};

	template <class... Args>
	struct kernel;

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
		class BinaryOp = std::plus<>>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, BinaryOp op = {}) const
		requires(std::indirectly_writable<Out, const std::iter_value_t<It>&> &&
			std::invocable<BinaryOp&, std::iter_value_t<It>, std::iter_reference_t<It>>)
	{
		return partial_sum[Traits::traits()](std::move(first), std::move(last),
			std::move(result), traits::fwd_fn(op));
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class BinaryOp = std::plus<>>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, BinaryOp op = {}) const
		requires(std::indirectly_writable<Out, const std::ranges::range_value_t<R>&> &&
			std::invocable<BinaryOp&, std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>>)
	{
		return partial_sum[Traits::traits()](std::forward<R>(r), std::move(result), traits::fwd_fn(op));
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
		class BinaryOp, class T>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, BinaryOp op, T init) const
		requires(std::indirectly_writable<Out, const T&> &&
			std::invocable<BinaryOp&, T, std::iter_reference_t<It>>)
	{
		auto src = get_source(std::move(first), std::move(last));
		auto dst = get_destination(std::move(result));
		auto work = init_kernel<decltype(src), decltype(dst), decltype(traits::fwd_fn(op)), T>(
			std::move(src), std::move(dst), traits::fwd_fn(op), std::move(init));
		work();
		return work.result();
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class BinaryOp, class T>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, BinaryOp op, T init) const
		requires(std::indirectly_writable<Out, const T&> &&
			std::invocable<BinaryOp&, T, std::ranges::range_reference_t<R>>)
	{
		auto src = get_source(std::forward<R>(r));
		auto dst = get_destination(std::move(result));
		auto work = init_kernel<decltype(src), decltype(dst), decltype(traits::fwd_fn(op)), T>(
			std::move(src), std::move(dst), traits::fwd_fn(op), std::move(init));
		work();
		return work.result();
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Computes an inclusive prefix scan over a range with an optional initial value.
 *
 * 1) Writes inclusive prefix sums of `[first, last)` under `op` (default `std::plus<>`) to `result`.
 *
 * 2) Same as (1), using range `r`.
 *
 * 3) Writes inclusive prefix scan of `[first, last)` seeded with `init` under `op` to `result`.
 *
 * 4) Same as (3), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel without init
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class BinaryOp = std::plus<> >
 * constexpr std::ranges::in_out_result<It, Out>
 * inclusive_scan( It first, Sent last, Out result, BinaryOp op = {} );
 *
 * // (2) Range without init
 * template< std::ranges::input_range R,
 *           std::weakly_incrementable Out, class BinaryOp = std::plus<> >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * inclusive_scan( R&& r, Out result, BinaryOp op = {} );
 *
 * // (3) Iterator-sentinel with init
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class BinaryOp, class T >
 * constexpr std::ranges::in_out_result<It, Out>
 * inclusive_scan( It first, Sent last, Out result, BinaryOp op, T init );
 *
 * // (4) Range with init
 * template< std::ranges::input_range R,
 *           std::weakly_incrementable Out, class BinaryOp, class T >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * inclusive_scan( R&& r, Out result, BinaryOp op, T init );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - input range of elements
 * - `result`             - beginning of destination range (may equal `first`)
 * - `op`                 - associative binary operation (defaults to `std::plus<>`)
 * - `init`               - optional initial value prepended to the scan
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
 * #include <raze/algorithm/numeric/InclusiveScan.h>
 *
 * int main() {
 *     std::vector<int> v{3, 1, 4, 1, 5};
 *     raze::algorithm::inclusive_scan(v, v.begin());
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::exclusive_scan`
 * - `raze::algorithm::transform_inclusive_scan`
 * - `raze::algorithm::partial_sum`
 */
constexpr inline auto inclusive_scan = options::function_with_traits<inclusive_scan_t>;

__RAZE_ALGORITHM_NAMESPACE_END

