#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct reduce_t : Traits, dispatchable<reduce_t<Traits>> {
	template <source Source, class T, class BinaryOp>
	struct kernel {
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
		raze_no_unique_address BinaryOp _op;

		constexpr explicit kernel(Source&& src, T init, BinaryOp op)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _init(std::move(init)),
			  _op(std::move(op))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it != _sent; ++_it)
				_init = std::invoke(_op, std::move(_init), *_it);
		}

		raze_nodiscard constexpr raze_always_inline T result() {
			return std::move(_init);
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent>
	raze_nodiscard constexpr raze_always_inline std::iter_value_t<It>
	operator()(It first, Sent last) const
		requires(std::default_initializable<std::iter_value_t<It>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::iter_value_t<It>{}, std::plus<>{});
	}

	template <std::ranges::input_range R>
	raze_nodiscard constexpr raze_always_inline std::ranges::range_value_t<R>
	operator()(R&& r) const
		requires(std::default_initializable<std::ranges::range_value_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			std::ranges::range_value_t<R>{}, std::plus<>{});
	}

	template <std::input_iterator It, std::sentinel_for<It> Sent,
		class T = std::iter_value_t<It>, class BinaryOp = std::plus<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(It first, Sent last, T init, BinaryOp op = {}) const
		requires(std::invocable<BinaryOp&, T, std::iter_reference_t<It>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::move(init), traits::fwd_fn(op));
	}

	template <std::ranges::input_range R,
		class T = std::ranges::range_value_t<R>, class BinaryOp = std::plus<>>
	raze_nodiscard constexpr raze_always_inline T
	operator()(R&& r, T init, BinaryOp op = {}) const
		requires(std::invocable<BinaryOp&, T, std::ranges::range_reference_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			std::move(init), traits::fwd_fn(op));
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Reduces a range of elements using a binary associative/commutative operation.
 *
 * 1) Reduces `[first, last)` starting from `std::iter_value_t<It>{}` using `std::plus<>`.
 *
 * 2) Same as (1), using range `r`.
 *
 * 3) Reduces `[first, last)` starting from `init` using `op`.
 *
 * 4) Same as (3), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel default sum overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent >
 * constexpr std::iter_value_t<It> reduce( It first, Sent last );
 *
 * // (2) Range default sum overload
 * template< std::ranges::input_range R >
 * constexpr std::ranges::range_value_t<R> reduce( R&& r );
 *
 * // (3) Iterator-sentinel custom init/op overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It>, class BinaryOp = std::plus<> >
 * constexpr T reduce( It first, Sent last, T init, BinaryOp op = {} );
 *
 * // (4) Range custom init/op overload
 * template< std::ranges::input_range R,
 *           class T = std::ranges::range_value_t<R>, class BinaryOp = std::plus<> >
 * constexpr T reduce( R&& r, T init, BinaryOp op = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - input range of elements
 * - `r`             - input range of elements
 * - `init`          - initial value
 * - `op`            - associative and commutative binary reduction operation (defaults to `std::plus<>`)
 *
 * ### Return value
 * The reduced value.
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
 * #include <raze/algorithm/numeric/Reduce.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5};
 *     std::cout << raze::algorithm::reduce(v) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::accumulate`
 * - `raze::algorithm::transform_reduce`
 */
constexpr inline auto reduce = options::function_with_traits<reduce_t>;

__RAZE_ALGORITHM_NAMESPACE_END

