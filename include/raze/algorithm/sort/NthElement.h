#pragma once

#include <raze/algorithm/sort/Sort.h>
#include <raze/algorithm/sort/PartialSort.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct nth_element_t : Traits, dispatchable<nth_element_t<Traits>> {
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
		unchecked_iterator_type _nth;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _last;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, iterator_type nth, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _nth(source_type::unwrap(std::move(nth))),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_nth, _sent)),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			if (_first == _last || _nth == _last) return;

			auto lo = _first;
			auto hi = _last;
			std::size_t depth = std::bit_width(static_cast<std::size_t>(hi - lo)) * 2;

			while (hi - lo > 3) {
				if (depth == 0) {
					partial_sort[options::drop_alignment<Traits>()](
						lo, _nth + 1, hi, traits::fwd_fn(_comp), traits::fwd_fn(_proj));
					return;
				}
				--depth;

				auto pivot = detail::unguarded_partition_pivot(lo, hi, _comp, _proj);
				if (pivot == _nth) return;
				if (pivot < _nth) lo = pivot + 1;
				else hi = pivot;
			}

			detail::insertion_sort(lo, hi, _comp, _proj);
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_last));
		}
	};

	template <std::random_access_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline It
	operator()(It first, It nth, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<It, Comp, Proj>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::move(nth), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::random_access_range R,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, std::ranges::iterator_t<R> nth, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			std::move(nth), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_sort
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Partially sorts a range so that the `nth` element is in its final sorted position (introselect).
 *
 * 1) Rearranges `[first, last)` such that the element at `nth` is the element that would occur there
 *    if the range were sorted with respect to `comp` and `proj`, and no element in `[first, nth)` is
 *    greater than any element in `[nth, last)`.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `nth_element(std::ranges::begin(r), nth, std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * nth_element( It first, It nth, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * nth_element( R&& r, std::ranges::iterator_t<R> nth, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range to reorder
 * - `r`             - the random-access range to reorder
 * - `nth`           - iterator to the target rank position
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear on average (`O(N)` comparisons), with `O(N log N)` worst-case fallback via `partial_sort`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/sort/NthElement.h>
 *
 * int main() {
 *     std::vector<int> v{5, 6, 4, 3, 2, 6, 7, 9, 3};
 *     auto mid = v.begin() + v.size() / 2;
 *     raze::algorithm::nth_element(v, mid);
 *     std::cout << "Median: " << *mid << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partial_sort`
 * - `raze::algorithm::sort`
 */
constexpr inline auto nth_element = options::function_with_traits<nth_element_t>;

__RAZE_ALGORITHM_NAMESPACE_END
