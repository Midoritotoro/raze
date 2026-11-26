#pragma once

#include <raze/algorithm/sort/Sort.h>
#include <raze/algorithm/set/InplaceMerge.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class Traits, class It, class Comp, class Proj>
constexpr void stable_sort_impl(It first, It last, Comp& comp, Proj& proj) {
	constexpr std::ptrdiff_t insertion_threshold = 16;

	const auto n = last - first;
	if (n <= insertion_threshold) {
		insertion_sort(first, last, comp, proj);
		return;
	}

	auto mid = first + (n >> 1);
	stable_sort_impl<Traits>(first, mid, comp, proj);
	stable_sort_impl<Traits>(mid, last, comp, proj);
	inplace_merge_impl<Traits>(first, mid, last, comp, proj);
}

} // namespace detail

template <class Traits>
struct stable_sort_t : Traits, dispatchable<stable_sort_t<Traits>> {
	template <source Source, class Comp, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _last;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_first, _sent)),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			if (_last - _first <= 1) return;
			detail::stable_sort_impl<Traits>(_first, _last, _comp, _proj);
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_last));
		}
	};

	template <std::random_access_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline It
	operator()(It first, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<It, Comp, Proj>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::random_access_range R,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_sort
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Sorts elements in a random-access range while preserving relative order of equivalent elements (C++26 P2562 `constexpr`).
 *
 * 1) Sorts `[first, last)` in ascending order with respect to `comp` and `proj` using stable
 *    merge sort, guaranteeing that equivalent elements retain their original relative order.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `stable_sort(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * stable_sort( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * stable_sort( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range to sort
 * - `r`             - the random-access range to sort
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * `O(N log N)` comparisons when auxiliary memory is available; falls back to `O(N log^2 N)`
 * in-place merges during constant evaluation (`if consteval`) or if memory allocation fails.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implements C++26 P2562 `constexpr` support in C++23.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <utility>
 * #include <vector>
 * #include <raze/algorithm/sort/StableSort.h>
 *
 * int main() {
 *     std::vector<std::pair<int, char>> v{{2, 'a'}, {1, 'b'}, {2, 'c'}, {1, 'd'}};
 *     raze::algorithm::stable_sort(v, {}, &std::pair<int, char>::first);
 *     for (auto [k, c] : v) std::cout << k << c << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::sort`
 * - `raze::algorithm::inplace_merge`
 */
constexpr inline auto stable_sort = options::function_with_traits<stable_sort_t>;

__RAZE_ALGORITHM_NAMESPACE_END

