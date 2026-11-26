#pragma once

#include <raze/algorithm/heap/MakeHeap.h>
#include <raze/algorithm/heap/SortHeap.h>
#include <bit>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class It, class Comp, class Proj>
constexpr void insertion_sort(It first, It last, Comp& comp, Proj& proj) {
	if (first == last) return;

	for (auto it = std::ranges::next(first); it != last; ++it) {
		auto prev = std::ranges::prev(it);
		if (std::invoke(comp, std::invoke(proj, *it), std::invoke(proj, *prev))) {
			auto val = std::ranges::iter_move(it);
			auto hole = it;
			do {
				*hole = std::ranges::iter_move(prev);
				hole = prev;
				if (hole == first) break;
				--prev;
			} while (std::invoke(comp, std::invoke(proj, val), std::invoke(proj, *prev)));
			*hole = std::move(val);
		}
	}
}

template <class It, class Comp, class Proj>
constexpr void sort3(It a, It b, It c, Comp& comp, Proj& proj) {
	if (std::invoke(comp, std::invoke(proj, *b), std::invoke(proj, *a)))
		std::ranges::iter_swap(a, b);
	if (std::invoke(comp, std::invoke(proj, *c), std::invoke(proj, *b))) {
		std::ranges::iter_swap(b, c);
		if (std::invoke(comp, std::invoke(proj, *b), std::invoke(proj, *a)))
			std::ranges::iter_swap(a, b);
	}
}

template <class It, class Comp, class Proj>
constexpr It unguarded_partition_pivot(It first, It last, Comp& comp, Proj& proj) {
	auto mid = first + ((last - first) >> 1);
	auto back = last - 1;
	sort3(first, mid, back, comp, proj);

	// Place median at `first + 1`; `*first <= *mid <= *back` act as sentinels.
	auto pivot_it = first + 1;
	std::ranges::iter_swap(pivot_it, mid);

	auto i = pivot_it;
	auto j = back;

	for (;;) {
		do { ++i; } while (std::invoke(comp, std::invoke(proj, *i), std::invoke(proj, *pivot_it)));
		do { --j; } while (std::invoke(comp, std::invoke(proj, *pivot_it), std::invoke(proj, *j)));
		if (i >= j) break;
		std::ranges::iter_swap(i, j);
	}

	std::ranges::iter_swap(pivot_it, j);
	return j;
}

template <class It, class Comp, class Proj>
constexpr void introsort_loop(It first, It last, std::size_t depth_limit, Comp& comp, Proj& proj) {
	constexpr std::ptrdiff_t insertion_threshold = 16;

	while (last - first > insertion_threshold) {
		if (depth_limit == 0) {
			const auto n = last - first;
			for (auto i = (n - 2) >> 1; ; --i) {
				heap_sift_down(first, n, i, comp, proj);
				if (i == 0) break;
			}
			for (auto m = n; m > 1; --m) {
				std::ranges::iter_swap(first, first + (m - 1));
				heap_sift_down(first, m - 1, decltype(m)(0), comp, proj);
			}
			return;
		}
		--depth_limit;

		auto pivot = unguarded_partition_pivot(first, last, comp, proj);
		if (pivot - first < last - (pivot + 1)) {
			introsort_loop(first, pivot, depth_limit, comp, proj);
			first = pivot + 1;
		}
		else {
			introsort_loop(pivot + 1, last, depth_limit, comp, proj);
			last = pivot;
		}
	}

	insertion_sort(first, last, comp, proj);
}

} // namespace detail

template <class Traits>
struct sort_t : Traits, dispatchable<sort_t<Traits>> {
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
			const auto n = _last - _first;
			if (n <= 1) return;

			const std::size_t depth = std::bit_width(static_cast<std::size_t>(n)) * 2;
			detail::introsort_loop(_first, _last, depth, _comp, _proj);
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
 * @brief Sorts elements in a random-access range into ascending order (unstable introsort).
 *
 * 1) Sorts `[first, last)` in-place with respect to `comp` and `proj` using introspective sort
 *    (median-of-3 quicksort with heapsort fallback and insertion sort for small partitions).
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `sort(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * sort( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * sort( R&& r, Comp comp = {}, Proj proj = {} );
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
 * Guaranteed `O(N log N)` worst-case comparisons and `O(log N)` auxiliary stack space.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/sort/Sort.h>
 *
 * int main() {
 *     std::vector<int> v{5, 2, 9, 1, 5, 6};
 *     raze::algorithm::sort(v);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::stable_sort`
 * - `raze::algorithm::partial_sort`
 * - `raze::algorithm::nth_element`
 * - `raze::algorithm::is_sorted`
 */
constexpr inline auto sort = options::function_with_traits<sort_t>;

__RAZE_ALGORITHM_NAMESPACE_END

