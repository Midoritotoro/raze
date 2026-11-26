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
struct transform_inclusive_scan_t : Traits, dispatchable<transform_inclusive_scan_t<Traits>> {
	template <source Source, destination Destination, class BinaryOp, class UnaryOp>
	struct no_init_kernel {
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
		raze_no_unique_address BinaryOp _bop;
		raze_no_unique_address UnaryOp _uop;

		constexpr explicit no_init_kernel(Source&& src, Destination&& dst, BinaryOp bop, UnaryOp uop)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _bop(std::move(bop)),
			  _uop(std::move(uop))
		{}

		raze_always_inline constexpr void operator()() {
			if (_in == _sent) return;

			auto acc = std::invoke(_uop, *_in);
			*_out = acc;
			++_out;
			for (++_in; _in != _sent; ++_in, ++_out) {
				acc = std::invoke(_bop, std::move(acc), std::invoke(_uop, *_in));
				*_out = acc;
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::in_out_result<iterator_type, out_iterator_type> result() {
			return { _source.wrap(std::move(_in)), _dest.wrap(std::move(_out)) };
		}
	};

	template <source Source, destination Destination, class BinaryOp, class UnaryOp, class T>
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
		raze_no_unique_address BinaryOp _bop;
		raze_no_unique_address UnaryOp _uop;
		T _init;

		constexpr explicit init_kernel(Source&& src, Destination&& dst, BinaryOp bop, UnaryOp uop, T init)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _bop(std::move(bop)),
			  _uop(std::move(uop)),
			  _init(std::move(init))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _in != _sent; ++_in, ++_out) {
				_init = std::invoke(_bop, std::move(_init), std::invoke(_uop, *_in));
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
		class BinaryOp, class UnaryOp>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, BinaryOp bop, UnaryOp uop) const
		requires(std::invocable<UnaryOp&, std::iter_reference_t<It>> &&
			std::invocable<BinaryOp&,
				std::decay_t<std::invoke_result_t<UnaryOp&, std::iter_reference_t<It>>>,
				std::invoke_result_t<UnaryOp&, std::iter_reference_t<It>>>)
	{
		auto src = get_source(std::move(first), std::move(last));
		auto dst = get_destination(std::move(result));
		auto work = no_init_kernel<decltype(src), decltype(dst),
			decltype(traits::fwd_fn(bop)), decltype(traits::fwd_fn(uop))>(
			std::move(src), std::move(dst), traits::fwd_fn(bop), traits::fwd_fn(uop));
		work();
		return work.result();
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class BinaryOp, class UnaryOp>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, BinaryOp bop, UnaryOp uop) const
		requires(std::invocable<UnaryOp&, std::ranges::range_reference_t<R>> &&
			std::invocable<BinaryOp&,
				std::decay_t<std::invoke_result_t<UnaryOp&, std::ranges::range_reference_t<R>>>,
				std::invoke_result_t<UnaryOp&, std::ranges::range_reference_t<R>>>)
	{
		auto src = get_source(std::forward<R>(r));
		auto dst = get_destination(std::move(result));
		auto work = no_init_kernel<decltype(src), decltype(dst),
			decltype(traits::fwd_fn(bop)), decltype(traits::fwd_fn(uop))>(
			std::move(src), std::move(dst), traits::fwd_fn(bop), traits::fwd_fn(uop));
		work();
		return work.result();
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
		class BinaryOp, class UnaryOp, class T>
	constexpr raze_always_inline std::ranges::in_out_result<It, Out>
	operator()(It first, Sent last, Out result, BinaryOp bop, UnaryOp uop, T init) const
		requires(std::invocable<UnaryOp&, std::iter_reference_t<It>> &&
			std::invocable<BinaryOp&, T, std::invoke_result_t<UnaryOp&, std::iter_reference_t<It>>>)
	{
		auto src = get_source(std::move(first), std::move(last));
		auto dst = get_destination(std::move(result));
		auto work = init_kernel<decltype(src), decltype(dst),
			decltype(traits::fwd_fn(bop)), decltype(traits::fwd_fn(uop)), T>(
			std::move(src), std::move(dst), traits::fwd_fn(bop), traits::fwd_fn(uop), std::move(init));
		work();
		return work.result();
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class BinaryOp, class UnaryOp, class T>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, BinaryOp bop, UnaryOp uop, T init) const
		requires(std::invocable<UnaryOp&, std::ranges::range_reference_t<R>> &&
			std::invocable<BinaryOp&, T, std::invoke_result_t<UnaryOp&, std::ranges::range_reference_t<R>>>)
	{
		auto src = get_source(std::forward<R>(r));
		auto dst = get_destination(std::move(result));
		auto work = init_kernel<decltype(src), decltype(dst),
			decltype(traits::fwd_fn(bop)), decltype(traits::fwd_fn(uop)), T>(
			std::move(src), std::move(dst), traits::fwd_fn(bop), traits::fwd_fn(uop), std::move(init));
		work();
		return work.result();
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Transforms each element via a unary operation and computes an inclusive prefix scan.
 *
 * 1) Applies `uop` to every element of `[first, last)` and computes an inclusive prefix scan using `bop`.
 *
 * 2) Same as (1), using range `r`.
 *
 * 3) Same as (1), seeded with initial value `init` (not transformed by `uop`).
 *
 * 4) Same as (3), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel without init
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class BinaryOp, class UnaryOp >
 * constexpr std::ranges::in_out_result<It, Out>
 * transform_inclusive_scan( It first, Sent last, Out result, BinaryOp bop, UnaryOp uop );
 *
 * // (2) Range without init
 * template< std::ranges::input_range R, std::weakly_incrementable Out,
 *           class BinaryOp, class UnaryOp >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * transform_inclusive_scan( R&& r, Out result, BinaryOp bop, UnaryOp uop );
 *
 * // (3) Iterator-sentinel with init
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class BinaryOp, class UnaryOp, class T >
 * constexpr std::ranges::in_out_result<It, Out>
 * transform_inclusive_scan( It first, Sent last, Out result, BinaryOp bop, UnaryOp uop, T init );
 *
 * // (4) Range with init
 * template< std::ranges::input_range R, std::weakly_incrementable Out,
 *           class BinaryOp, class UnaryOp, class T >
 * constexpr std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
 * transform_inclusive_scan( R&& r, Out result, BinaryOp bop, UnaryOp uop, T init );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - input range
 * - `result`             - beginning of destination range (may equal `first`)
 * - `bop`                - binary scan operation
 * - `uop`                - unary element transform
 * - `init`               - optional initial accumulator value
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
 * #include <raze/algorithm/numeric/TransformInclusiveScan.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4};
 *     std::vector<int> out(v.size());
 *     raze::algorithm::transform_inclusive_scan(v, out.begin(), std::plus<>{}, [](int x) { return x * 10; });
 *     for (int x : out) std::cout << x << ' ';
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
constexpr inline auto transform_inclusive_scan = options::function_with_traits<transform_inclusive_scan_t>;

__RAZE_ALGORITHM_NAMESPACE_END

