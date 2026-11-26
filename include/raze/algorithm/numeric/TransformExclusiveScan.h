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
struct transform_exclusive_scan_t : Traits, dispatchable<transform_exclusive_scan_t<Traits>> {
	template <source Source, destination Destination, class T, class BinaryOp, class UnaryOp>
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
		raze_no_unique_address BinaryOp _bop;
		raze_no_unique_address UnaryOp _uop;

		constexpr explicit kernel(Source&& src, Destination&& dst, T init, BinaryOp bop, UnaryOp uop)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _init(std::move(init)),
			  _bop(std::move(bop)),
			  _uop(std::move(uop))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _in != _sent; ++_in, ++_out) {
				auto next = std::invoke(_bop, _init, std::invoke(_uop, *_in));
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
		class T, class BinaryOp, class UnaryOp>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, T init, BinaryOp bop, UnaryOp uop) const
		requires(std::indirectly_writable<Out, T> &&
			std::invocable<UnaryOp&, std::iter_reference_t<It>> &&
			std::invocable<BinaryOp&, const T&, std::invoke_result_t<UnaryOp&, std::iter_reference_t<It>>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(result)), std::move(init),
			traits::fwd_fn(bop), traits::fwd_fn(uop));
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class T, class BinaryOp, class UnaryOp>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, T init, BinaryOp bop, UnaryOp uop) const
		requires(std::indirectly_writable<Out, T> &&
			std::invocable<UnaryOp&, std::ranges::range_reference_t<R>> &&
			std::invocable<BinaryOp&, const T&, std::invoke_result_t<UnaryOp&, std::ranges::range_reference_t<R>>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(result)), std::move(init),
			traits::fwd_fn(bop), traits::fwd_fn(uop));
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Transforms each element via a unary operation and computes an exclusive prefix scan starting from `init`.
 *
 * 1) Writes to `result[i]` the accumulation of `init` and `uop(elem)` for elements `[0, i)` under `bop`
 *    (excluding element `i` itself). Safe when `result == first` (in-place).
 *
 * 2) Same as (1), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class T, class BinaryOp, class UnaryOp >
 * constexpr std::ranges::in_out_result<It, Out>
 * transform_exclusive_scan( It first, Sent last, Out result, T init, BinaryOp bop, UnaryOp uop );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable Out,
 *           class T, class BinaryOp, class UnaryOp >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * transform_exclusive_scan( R&& r, Out result, T init, BinaryOp bop, UnaryOp uop );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - input range
 * - `result`             - beginning of destination range (may equal `first`)
 * - `init`               - initial value written at index 0 (not transformed by `uop`)
 * - `bop`                - binary scan operation
 * - `uop`                - unary element transform
 *
 * ### Return value
 * A `std::ranges::in_out_result` `{in, out}`.
 *
 * ### Complexity
 * Linear: `O(N)` applications of `uop` and `bop`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <functional>
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/numeric/TransformExclusiveScan.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4};
 *     std::vector<int> out(v.size());
 *     raze::algorithm::transform_exclusive_scan(v, out.begin(), 0, std::plus<>{}, [](int x) { return x * 10; });
 *     for (int x : out) std::cout << x << ' ';
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
 */
constexpr inline auto transform_exclusive_scan = options::function_with_traits<transform_exclusive_scan_t>;

__RAZE_ALGORITHM_NAMESPACE_END

