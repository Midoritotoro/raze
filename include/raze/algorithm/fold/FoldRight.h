#pragma once

#include <raze/algorithm/fold/FoldLeftWithIter.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fold_right_t : Traits, dispatchable<fold_right_t<Traits>> {
	template <source Source, class T, class F>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using result_value_type = std::decay_t<std::invoke_result_t<F&, std::iter_reference_t<unchecked_iterator_type>, T>>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		T _init;
		raze_no_unique_address F _f;
		std::optional<result_value_type> _accum;

		constexpr explicit kernel(Source&& src, T init, F f)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _init(std::move(init)),
			  _f(std::move(f))
		{}

		raze_always_inline constexpr void operator()() {
			if (_first == _sent) {
				_accum.emplace(std::move(_init));
				return;
			}

			auto tail = std::ranges::next(_first, _sent);
			--tail;
			_accum.emplace(std::invoke(_f, *tail, std::move(_init)));
			while (tail != _first) {
				--tail;
				*_accum = std::invoke(_f, *tail, std::move(*_accum));
			}
		}

		raze_nodiscard constexpr raze_always_inline result_value_type result() {
			return std::move(*_accum);
		}
	};

	template <std::bidirectional_iterator It, std::sentinel_for<It> Sent,
		class T = std::iter_value_t<It>, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(It first, Sent last, T init, F f) const
		requires(detail::indirectly_binary_right_foldable<F, T, It>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::move(init), std::move(f));
	}

	template <std::ranges::bidirectional_range R,
		class T = std::ranges::range_value_t<R>, class F>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(R&& r, T init, F f) const
		requires(detail::indirectly_binary_right_foldable<F, T, std::ranges::iterator_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			std::move(init), std::move(f));
	}
};

/**
 * @ingroup raze_algo_fold
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Right-folds the elements of a bidirectional range starting from an initial value (C++23).
 *
 * 1) Computes `f(x_0, f(x_1, ... f(x_{n-1}, init)...))` across `[first, last)` from right to left.
 *
 * 2) Same as (1), but uses `r` as the source range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It>, class F >
 * constexpr auto
 * fold_right( It first, Sent last, T init, F f );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R,
 *           class T = std::ranges::range_value_t<R>, class F >
 * constexpr auto
 * fold_right( R&& r, T init, F f );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - bidirectional range to fold
 * - `r`             - bidirectional range to fold
 * - `init`          - initial accumulator value (supports braced-init lists via C++26 P2248)
 * - `f`             - binary fold callable `f(elem, accum)`
 *
 * ### Return value
 * The final right-folded value.
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
 * #include <raze/algorithm/fold/FoldRight.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3};
 *     std::cout << raze::algorithm::fold_right(v, 0, std::minus<>{}) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fold_right_last`
 * - `raze::algorithm::fold_left`
 */
constexpr inline auto fold_right = options::function_with_traits<fold_right_t>;

__RAZE_ALGORITHM_NAMESPACE_END

