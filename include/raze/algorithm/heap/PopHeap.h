#pragma once

#include <raze/algorithm/heap/PushHeap.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct pop_heap_t : Traits, dispatchable<pop_heap_t<Traits>> {
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
			if (n > 1) {
				std::ranges::iter_swap(_first, _first + (n - 1));
				detail::heap_sift_down(_first, n - 1, decltype(n)(0), _comp, _proj);
			}
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
 * @brief Moves the largest element from a max-heap to the end of the range and restores the heap.
 *
 * 1) Swaps `*first` with `*(last - 1)` and restores the max-heap property on `[first, last - 1)`
 *    with respect to `comp` and `proj`.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `pop_heap(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * pop_heap( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * pop_heap( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access max-heap range
 * - `r`             - the random-access max-heap range
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Logarithmic: at most `2 * log2(N)` comparisons.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/heap/MakeHeap.h>
 * #include <raze/algorithm/heap/PopHeap.h>
 *
 * int main() {
 *     std::vector<int> v{3, 1, 4, 1, 5, 9};
 *     raze::algorithm::make_heap(v);
 *     raze::algorithm::pop_heap(v);
 *     std::cout << "Popped max: " << v.back() << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::push_heap`
 * - `raze::algorithm::make_heap`
 * - `raze::algorithm::sort_heap`
 */
constexpr inline auto pop_heap = options::function_with_traits<pop_heap_t>;

__RAZE_ALGORITHM_NAMESPACE_END

