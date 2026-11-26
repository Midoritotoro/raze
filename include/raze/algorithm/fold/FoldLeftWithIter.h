#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class F, class T, class It, class U>
concept indirectly_binary_left_foldable_impl =
	std::movable<T> &&
	std::movable<U> &&
	std::convertible_to<T, U> &&
	std::invocable<F&, U, std::iter_reference_t<It>> &&
	std::assignable_from<U&, std::invoke_result_t<F&, U, std::iter_reference_t<It>>>;

template <class F, class T, class It>
concept indirectly_binary_left_foldable =
	std::copy_constructible<F> &&
	std::indirectly_readable<It> &&
	std::invocable<F&, T, std::iter_reference_t<It>> &&
	std::convertible_to<std::invoke_result_t<F&, T, std::iter_reference_t<It>>,
		std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<It>>>> &&
	indirectly_binary_left_foldable_impl<F, T, It,
		std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<It>>>>;

template <class F>
struct flipped_binary_fn {
	F f;

	template <class T, class U>
		requires std::invocable<F&, U, T>
	constexpr std::invoke_result_t<F&, U, T> operator()(T&& t, U&& u) {
		return std::invoke(f, std::forward<U>(u), std::forward<T>(t));
	}
};

template <class F, class T, class It>
concept indirectly_binary_right_foldable =
	indirectly_binary_left_foldable<flipped_binary_fn<F>, T, It>;

} // namespace detail

template <class Traits>
struct fold_left_with_iter_t : Traits, dispatchable<fold_left_with_iter_t<Traits>> {
	template <source Source, class T, class F>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using result_value_type = std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<unchecked_iterator_type>>>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _it;
		raze_no_unique_address unchecked_sentinel_type _sent;
		T _init;
		raze_no_unique_address F _f;
		std::optional<result_value_type> _accum;

		constexpr explicit kernel(Source&& src, T init, F f)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _init(std::move(init)),
			  _f(std::move(f))
		{}

		raze_always_inline constexpr void operator()() {
			if (_it == _sent) {
				_accum.emplace(std::move(_init));
				return;
			}

			_accum.emplace(std::invoke(_f, std::move(_init), *_it));
			for (++_it; _it != _sent; ++_it)
				*_accum = std::invoke(_f, std::move(*_accum), *_it);
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::fold_left_with_iter_result<iterator_type, result_value_type> result() {
			return { _source.wrap(std::move(_it)), std::move(*_accum) };
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent,
		class T = std::iter_value_t<It>, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(It first, Sent last, T init, F f) const
		requires(detail::indirectly_binary_left_foldable<F, T, It>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::move(init), std::move(f));
	}

	template <std::ranges::input_range R,
		class T = std::ranges::range_value_t<R>, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(R&& r, T init, F f) const
		requires(detail::indirectly_binary_left_foldable<F, T, std::ranges::iterator_t<R>>)
	{
		auto res = this->dispatch(get_source(std::forward<R>(r)),
			std::move(init), std::move(f));
		if constexpr (std::ranges::borrowed_range<R>)
			return res;
		else
			return std::ranges::fold_left_with_iter_result<std::ranges::dangling, decltype(res.value)>{
				std::ranges::dangling{}, std::move(res.value)
			};
	}
};

/**
 * @ingroup raze_algo_fold
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Left-folds a range with an initial value and returns both the end iterator and accumulated value (C++23).
 *
 * 1) Left-folds `[first, last)` starting from `init` via `accum = std::invoke(f, std::move(accum), *i)`
 *    and returns `{last, std::move(accum)}`.
 *
 * 2) Same as (1), but uses `r` as the source range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It>, class F >
 * constexpr std::ranges::fold_left_with_iter_result<It, U>
 * fold_left_with_iter( It first, Sent last, T init, F f );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class T = std::ranges::range_value_t<R>, class F >
 * constexpr std::ranges::fold_left_with_iter_result<std::ranges::borrowed_iterator_t<R>, U>
 * fold_left_with_iter( R&& r, T init, F f );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - input range to fold
 * - `r`             - input range to fold
 * - `init`          - initial accumulator value (supports braced-init lists via C++26 P2248)
 * - `f`             - binary fold callable `f(accum, elem)`
 *
 * ### Return value
 * A `std::ranges::fold_left_with_iter_result` `{in, value}`.
 *
 * ### Complexity
 * Linear: exactly `N` invocations of `f`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <functional>
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/fold/FoldLeftWithIter.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4};
 *     auto res = raze::algorithm::fold_left_with_iter(v, 0, std::plus<>{});
 *     std::cout << res.value << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fold_left`
 * - `raze::algorithm::fold_left_first_with_iter`
 * - `raze::algorithm::fold_right`
 */
constexpr inline auto fold_left_with_iter = options::function_with_traits<fold_left_with_iter_t>;

__RAZE_ALGORITHM_NAMESPACE_END

