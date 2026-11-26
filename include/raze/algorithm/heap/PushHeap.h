#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class It, class Diff, class Comp, class Proj>
constexpr void heap_sift_up(It first, Diff hole, Comp& comp, Proj& proj) {
	if (hole <= 0) return;

	Diff parent = (hole - 1) >> 1;
	if (!std::invoke(comp, std::invoke(proj, first[parent]), std::invoke(proj, first[hole])))
		return;

	auto value = std::ranges::iter_move(first + hole);
	do {
		first[hole] = std::ranges::iter_move(first + parent);
		hole = parent;
		if (hole == 0) break;
		parent = (hole - 1) >> 1;
	} while (std::invoke(comp, std::invoke(proj, first[parent]), std::invoke(proj, value)));

	first[hole] = std::move(value);
}

template <class It, class Diff, class Comp, class Proj>
constexpr void heap_sift_down(It first, Diff len, Diff hole, Comp& comp, Proj& proj) {
	Diff child = (hole << 1) + 1;
	if (child >= len) return;

	if (child + 1 < len &&
		std::invoke(comp, std::invoke(proj, first[child]), std::invoke(proj, first[child + 1])))
	{
		++child;
	}

	if (!std::invoke(comp, std::invoke(proj, first[hole]), std::invoke(proj, first[child])))
		return;

	auto value = std::ranges::iter_move(first + hole);
	do {
		first[hole] = std::ranges::iter_move(first + child);
		hole = child;
		child = (hole << 1) + 1;
		if (child >= len) break;
		if (child + 1 < len &&
			std::invoke(comp, std::invoke(proj, first[child]), std::invoke(proj, first[child + 1])))
		{
			++child;
		}
	} while (std::invoke(comp, std::invoke(proj, value), std::invoke(proj, first[child])));

	first[hole] = std::move(value);
}

} // namespace detail

template <class Traits>
struct push_heap_t : Traits, dispatchable<push_heap_t<Traits>> {
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
			if (n > 1)
				detail::heap_sift_up(_first, n - 1, _comp, _proj);
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
 * @ingroup raze_algo_heap
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Inserts the last element of a range into the max-heap formed by the preceding elements.
 *
 * 1) Inserts the element at `last - 1` into the max-heap `[first, last - 1)` so that `[first, last)`
 *    becomes a valid max-heap with respect to `comp` and `proj`.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `push_heap(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * push_heap( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * push_heap( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range where `[first, last - 1)` is a valid max-heap
 * - `r`             - the random-access range
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Logarithmic: at most `log2(N)` comparisons and moves.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/heap/MakeHeap.h>
 * #include <raze/algorithm/heap/PushHeap.h>
 *
 * int main() {
 *     std::vector<int> v{3, 1, 4, 1, 5};
 *     raze::algorithm::make_heap(v);
 *     v.push_back(9);
 *     raze::algorithm::push_heap(v);
 *     std::cout << "Top: " << v.front() << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::pop_heap`
 * - `raze::algorithm::make_heap`
 * - `raze::algorithm::sort_heap`
 */
constexpr inline auto push_heap = options::function_with_traits<push_heap_t>;

__RAZE_ALGORITHM_NAMESPACE_END

