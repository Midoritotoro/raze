#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct is_heap_until_t : Traits, dispatchable<is_heap_until_t<Traits>> {
	template <source Source, class Comp, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using difference_type = std::iter_difference_t<unchecked_iterator_type>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _result;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _result(_first),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			auto last = std::ranges::next(_first, _sent);
			const difference_type n = last - _first;

			difference_type parent = 0;
			for (difference_type child = 1; child < n; ++child) {
				if (std::invoke(_comp, std::invoke(_proj, _first[parent]), std::invoke(_proj, _first[child]))) {
					_result = _first + child;
					return;
				}
				if ((child & 1) == 0)
					++parent;
			}

			_result = last;
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_result));
		}
	};

	template <std::random_access_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline It
	operator()(It first, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::random_access_range R,
		class Comp = std::less<>, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp,
			std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_heap
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the largest subrange beginning at the start of a range that forms a max-heap.
 *
 * 1) Examines `[first, last)` and returns the upper bound of the largest prefix `[first, i)` that
 *    satisfies the max-heap property with respect to `comp` and `proj`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `is_heap_until(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
 * constexpr It
 * is_heap_until( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * is_heap_until( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range to examine
 * - `r`             - the random-access range to examine
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * Iterator to the first element violating the heap property, or an iterator equal to `last`.
 *
 * ### Complexity
 * Linear: at most `max(N - 1, 0)` comparisons.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/heap/IsHeapUntil.h>
 *
 * int main() {
 *     const std::vector<int> v{9, 5, 4, 1, 1, 3, 2, 6};
 *     auto it = raze::algorithm::is_heap_until(v);
 *     std::cout << "Heap prefix length: " << (it - v.begin()) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::is_heap`
 * - `raze::algorithm::make_heap`
 */
constexpr inline auto is_heap_until = options::function_with_traits<is_heap_until_t>;

__RAZE_ALGORITHM_NAMESPACE_END

