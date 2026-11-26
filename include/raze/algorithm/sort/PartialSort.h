#pragma once

#include <raze/algorithm/heap/MakeHeap.h>
#include <raze/algorithm/heap/SortHeap.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct partial_sort_t : Traits, dispatchable<partial_sort_t<Traits>> {
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
		unchecked_iterator_type _middle;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _last;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, iterator_type middle, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _middle(source_type::unwrap(std::move(middle))),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_middle, _sent)),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			const auto k = _middle - _first;
			if (k == 0) return;

			if (k >= 2) {
				for (auto i = (k - 2) >> 1; ; --i) {
					detail::heap_sift_down(_first, k, i, _comp, _proj);
					if (i == 0) break;
				}
			}

			for (auto it = _middle; it != _last; ++it) {
				if (std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, *_first))) {
					std::ranges::iter_swap(it, _first);
					detail::heap_sift_down(_first, k, decltype(k)(0), _comp, _proj);
				}
			}

			for (auto m = k; m > 1; --m) {
				std::ranges::iter_swap(_first, _first + (m - 1));
				detail::heap_sift_down(_first, m - 1, decltype(m)(0), _comp, _proj);
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_last));
		}
	};

	template <std::random_access_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline It
	operator()(It first, It middle, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<It, Comp, Proj>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::move(middle), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::random_access_range R,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			std::move(middle), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_sort
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Partially sorts a range so that the first `middle - first` elements are the smallest in sorted order.
 *
 * 1) Rearranges `[first, last)` such that `[first, middle)` contains the smallest `middle - first`
 *    elements in ascending order with respect to `comp` and `proj`. The remaining elements in
 *    `[middle, last)` are left in an unspecified order.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `partial_sort(std::ranges::begin(r), middle, std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * partial_sort( It first, It middle, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * partial_sort( R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range to partially sort
 * - `r`             - the random-access range to partially sort
 * - `middle`        - iterator delimiting the sorted prefix `[first, middle)`
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Approximately `N * log2(K)` comparisons where `N = last - first` and `K = middle - first`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/sort/PartialSort.h>
 *
 * int main() {
 *     std::vector<int> v{9, 4, 7, 1, 8, 2, 6};
 *     raze::algorithm::partial_sort(v, v.begin() + 3);
 *     for (int i = 0; i < 3; ++i) std::cout << v[i] << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partial_sort_copy`
 * - `raze::algorithm::sort`
 * - `raze::algorithm::nth_element`
 */
constexpr inline auto partial_sort = options::function_with_traits<partial_sort_t>;

__RAZE_ALGORITHM_NAMESPACE_END

