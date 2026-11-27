#pragma once

#include <raze/algorithm/fold/FoldLeftWithIter.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fold_right_last_t : Traits, dispatchable<fold_right_last_t<Traits>> {
	template <source Source, class F>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using result_value_type = std::decay_t<std::invoke_result_t<
			F&, std::iter_reference_t<unchecked_iterator_type>, std::iter_value_t<unchecked_iterator_type>>>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		raze_no_unique_address F _f;
		std::optional<result_value_type> _accum;

		constexpr explicit kernel(Source&& src, F f)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _f(std::move(f))
		{}

		constexpr bool exit() const noexcept {
			return _first == _sent;
		}

		raze_nodiscard constexpr std::optional<result_value_type> default_result() const noexcept {
			return std::nullopt;
		}

		raze_always_inline constexpr void operator()() {
			auto tail = std::ranges::next(_first, _sent);
			--tail;
			result_value_type acc(*tail);
			while (tail != _first) {
				--tail;
				acc = std::invoke(_f, *tail, std::move(acc));
			}
			_accum.emplace(std::move(acc));
		}

		raze_nodiscard constexpr raze_always_inline std::optional<result_value_type> result() {
			return std::move(_accum);
		}
	};

	template <std::bidirectional_iterator It, std::sentinel_for<It> Sent, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(It first, Sent last, F f) const
		requires(std::constructible_from<std::iter_value_t<It>, std::iter_reference_t<It>> &&
			detail::indirectly_binary_right_foldable<F, std::iter_value_t<It>, It>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), std::move(f));
	}

	template <std::ranges::bidirectional_range R, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(R&& r, F f) const
		requires(std::constructible_from<std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>> &&
			detail::indirectly_binary_right_foldable<F, std::ranges::range_value_t<R>, std::ranges::iterator_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), std::move(f));
	}
};

/**
 * @ingroup raze_algo_fold
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Right-folds a bidirectional range using its last element as the initial value (C++23).
 *
 * 1) Returns `std::optional` containing `f(x_0, f(x_1, ... f(x_{n-2}, x_{n-1})...))`, or `std::nullopt`
 *    if `[first, last)` is empty.
 *
 * 2) Same as (1), but uses `r` as the source range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent, class F >
 * constexpr std::optional<U>
 * fold_right_last( It first, Sent last, F f );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R, class F >
 * constexpr std::optional<U>
 * fold_right_last( R&& r, F f );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - bidirectional range to fold
 * - `r`             - bidirectional range to fold
 * - `f`             - binary fold callable `f(elem, accum)`
 *
 * ### Return value
 * `std::optional<U>` holding the right-folded value, or `std::nullopt` if the range is empty.
 *
 * ### Complexity
 * Linear: `max(N - 1, 0)` invocations of `f`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <functional>
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/fold/FoldRightLast.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3};
 *     if (auto r = raze::algorithm::fold_right_last(v, std::minus<>{}))
 *         std::cout << *r << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fold_right`
 * - `raze::algorithm::fold_left_first`
 */
constexpr inline auto fold_right_last = options::function_with_traits<fold_right_last_t>;

__RAZE_ALGORITHM_NAMESPACE_END

