#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct transform_reduce_t : Traits, dispatchable<transform_reduce_t<Traits>> {
	template <source Source, class T, class ReduceOp, class TransformOp>
	struct unary_kernel {
		using source_type = std::remove_cvref_t<Source>;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _it;
		raze_no_unique_address unchecked_sentinel_type _sent;
		T _init;
		raze_no_unique_address ReduceOp _reduce;
		raze_no_unique_address TransformOp _transform;

		constexpr explicit unary_kernel(Source&& src, T init, ReduceOp red, TransformOp trans)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _init(std::move(init)),
			  _reduce(std::move(red)),
			  _transform(std::move(trans))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it != _sent; ++_it)
				_init = std::invoke(_reduce, std::move(_init), std::invoke(_transform, *_it));
		}

		raze_nodiscard constexpr raze_always_inline T result() {
			return std::move(_init);
		}
	};

	template <source Source1, class It2, class T, class ReduceOp, class TransformOp>
	struct binary_iter_kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source1_type _source1;
		unchecked_iterator1_type _it1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		It2 _it2;
		T _init;
		raze_no_unique_address ReduceOp _reduce;
		raze_no_unique_address TransformOp _transform;

		constexpr explicit binary_iter_kernel(Source1&& s1, It2 it2, T init, ReduceOp red, TransformOp trans)
			: _source1(std::forward<Source1>(s1)),
			  _it1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _it2(std::move(it2)),
			  _init(std::move(init)),
			  _reduce(std::move(red)),
			  _transform(std::move(trans))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it1 != _sent1; ++_it1, ++_it2)
				_init = std::invoke(_reduce, std::move(_init), std::invoke(_transform, *_it1, *_it2));
		}

		raze_nodiscard constexpr raze_always_inline T result() {
			return std::move(_init);
		}
	};

	template <source Source1, source Source2, class T, class ReduceOp, class TransformOp>
	struct binary_range_kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;
		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source1_type _source1;
		raze_no_unique_address source2_type _source2;
		unchecked_iterator1_type _it1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _it2;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		T _init;
		raze_no_unique_address ReduceOp _reduce;
		raze_no_unique_address TransformOp _transform;

		constexpr explicit binary_range_kernel(Source1&& s1, Source2&& s2, T init, ReduceOp red, TransformOp trans)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _it1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _it2(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _init(std::move(init)),
			  _reduce(std::move(red)),
			  _transform(std::move(trans))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it1 != _sent1 && _it2 != _sent2; ++_it1, ++_it2)
				_init = std::invoke(_reduce, std::move(_init), std::invoke(_transform, *_it1, *_it2));
		}

		raze_nodiscard constexpr raze_always_inline T result() {
			return std::move(_init);
		}
	};

	template <class... Args>
	struct kernel;

	template <std::input_iterator It, std::sentinel_for<It> Sent,
		class T, class BinaryOp, class UnaryOp>
	raze_nodiscard constexpr raze_always_inline T
	operator()(It first, Sent last, T init, BinaryOp reduce_op, UnaryOp transform_op) const
		requires(std::invocable<UnaryOp&, std::iter_reference_t<It>> &&
			std::invocable<BinaryOp&, T, std::invoke_result_t<UnaryOp&, std::iter_reference_t<It>>>)
	{
		auto src = get_source(std::move(first), std::move(last));
		auto work = unary_kernel<decltype(src), T, decltype(traits::fwd_fn(reduce_op)),
			decltype(traits::fwd_fn(transform_op))>(
			std::move(src), std::move(init), traits::fwd_fn(reduce_op), traits::fwd_fn(transform_op));
		work();
		return work.result();
	}

	template <std::ranges::input_range R, class T, class BinaryOp, class UnaryOp>
	raze_nodiscard constexpr raze_always_inline T
	operator()(R&& r, T init, BinaryOp reduce_op, UnaryOp transform_op) const
		requires(std::invocable<UnaryOp&, std::ranges::range_reference_t<R>> &&
			std::invocable<BinaryOp&, T, std::invoke_result_t<UnaryOp&, std::ranges::range_reference_t<R>>>)
	{
		auto src = get_source(std::forward<R>(r));
		auto work = unary_kernel<decltype(src), T, decltype(traits::fwd_fn(reduce_op)),
			decltype(traits::fwd_fn(transform_op))>(
			std::move(src), std::move(init), traits::fwd_fn(reduce_op), traits::fwd_fn(transform_op));
		work();
		return work.result();
	}

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1, std::input_iterator It2,
		class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(It1 first1, Sent1 last1, It2 first2, T init,
		BinaryOp1 reduce_op = {}, BinaryOp2 transform_op = {}) const
			requires(!std::sentinel_for<T, It2> &&
				std::invocable<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>> &&
				std::invocable<BinaryOp1&, T,
					std::invoke_result_t<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>>>)
	{
		auto src1 = get_source(std::move(first1), std::move(last1));
		auto work = binary_iter_kernel<decltype(src1), It2, T, decltype(traits::fwd_fn(reduce_op)),
			decltype(traits::fwd_fn(transform_op))>(
			std::move(src1), std::move(first2), std::move(init),
			traits::fwd_fn(reduce_op), traits::fwd_fn(transform_op));
		work();
		return work.result();
	}

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
		std::input_iterator It2, std::sentinel_for<It2> Sent2,
		class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2, T init,
		BinaryOp1 reduce_op = {}, BinaryOp2 transform_op = {}) const
			requires(std::invocable<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>> &&
				std::invocable<BinaryOp1&, T,
					std::invoke_result_t<BinaryOp2&, std::iter_reference_t<It1>, std::iter_reference_t<It2>>>)
	{
		auto src1 = get_source(std::move(first1), std::move(last1));
		auto src2 = get_source(std::move(first2), std::move(last2));
		auto work = binary_range_kernel<decltype(src1), decltype(src2), T,
			decltype(traits::fwd_fn(reduce_op)), decltype(traits::fwd_fn(transform_op))>(
			std::move(src1), std::move(src2), std::move(init),
			traits::fwd_fn(reduce_op), traits::fwd_fn(transform_op));
		work();
		return work.result();
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2,
		class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(R1&& r1, R2&& r2, T init, BinaryOp1 reduce_op = {}, BinaryOp2 transform_op = {}) const
		requires(std::invocable<BinaryOp2&, std::ranges::range_reference_t<R1>, std::ranges::range_reference_t<R2>> &&
			std::invocable<BinaryOp1&, T,
				std::invoke_result_t<BinaryOp2&, std::ranges::range_reference_t<R1>, std::ranges::range_reference_t<R2>>>)
	{
		auto src1 = get_source(std::forward<R1>(r1));
		auto src2 = get_source(std::forward<R2>(r2));
		auto work = binary_range_kernel<decltype(src1), decltype(src2), T,
			decltype(traits::fwd_fn(reduce_op)), decltype(traits::fwd_fn(transform_op))>(
			std::move(src1), std::move(src2), std::move(init),
			traits::fwd_fn(reduce_op), traits::fwd_fn(transform_op));
		work();
		return work.result();
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Applies a unary or binary transformation and reduces the results into a single value.
 *
 * 1) Unary iterator-sentinel form: transforms each element of `[first, last)` via `transform_op`
 *    and reduces with `reduce_op` starting from `init`.
 *
 * 2) Unary range form: same as (1) using range `r`.
 *
 * 3) Binary iterator form (unbounded second range): transforms pairs `(*i1, *i2)` via `transform_op`
 *    (default `std::multiplies<>`) and reduces via `reduce_op` (default `std::plus<>`).
 *
 * 4) Binary iterator-sentinel form (two bounded ranges).
 *
 * 5) Binary range form using `r1` and `r2`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Unary iterator-sentinel
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T, class BinaryOp, class UnaryOp >
 * constexpr T transform_reduce( It first, Sent last, T init, BinaryOp reduce_op, UnaryOp transform_op );
 *
 * // (2) Unary range
 * template< std::ranges::input_range R, class T, class BinaryOp, class UnaryOp >
 * constexpr T transform_reduce( R&& r, T init, BinaryOp reduce_op, UnaryOp transform_op );
 *
 * // (3) Binary iterator (unbounded second sequence)
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1, std::input_iterator It2,
 *           class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<> >
 * constexpr T transform_reduce( It1 first1, Sent1 last1, It2 first2, T init,
 *                               BinaryOp1 reduce_op = {}, BinaryOp2 transform_op = {} );
 *
 * // (4) Binary iterator-sentinel (bounded)
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::input_iterator It2, std::sentinel_for<It2> Sent2,
 *           class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<> >
 * constexpr T transform_reduce( It1 first1, Sent1 last1, It2 first2, Sent2 last2, T init,
 *                               BinaryOp1 reduce_op = {}, BinaryOp2 transform_op = {} );
 *
 * // (5) Binary range
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           class T, class BinaryOp1 = std::plus<>, class BinaryOp2 = std::multiplies<> >
 * constexpr T transform_reduce( R1&& r1, R2&& r2, T init,
 *                               BinaryOp1 reduce_op = {}, BinaryOp2 transform_op = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r`             - input range (unary form)
 * - `first1`, `last1`, `first2`, `last2`, `r1`, `r2` - input ranges (binary form)
 * - `init`                           - initial accumulator value
 * - `reduce_op`                      - binary reduction operation
 * - `transform_op`                   - unary or binary transformation operation
 *
 * ### Return value
 * The reduced value of type `T`.
 *
 * ### Complexity
 * Linear: `O(N)` applications of `transform_op` and `reduce_op`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/numeric/TransformReduce.h>
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3}, b{4, 5, 6};
 *     std::cout << raze::algorithm::transform_reduce(a, b, 0) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::reduce`
 * - `raze::algorithm::inner_product`
 */
constexpr inline auto transform_reduce = options::function_with_traits<transform_reduce_t>;

__RAZE_ALGORITHM_NAMESPACE_END

