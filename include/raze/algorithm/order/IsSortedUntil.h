#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct is_sorted_until_t : Traits, dispatchable<is_sorted_until_t<Traits>> {
	template <source Source, class Comp, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;

		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
				!traits::is_iterator_volatile_v<iterator_type> &&
				vectorizable_binary_predicate<Comp, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;
		unchecked_iterator_type _in_it;
		raze_no_unique_address unchecked_sentinel_type _in_sent;

		constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
			: _source(std::forward<Source>(source)), _comp(comp), _proj(proj),
			  _in_it(_source.ubegin()), _in_sent(_source.uend())
		{}

		raze_always_inline constexpr void operator()() {
			if (_in_it == _in_sent) return;

			auto prev = _in_it;
			raze_disable_unrolling
			for (++_in_it; _in_it != _in_sent; ++_in_it, ++prev) {
				if (std::invoke(_comp, std::invoke(_proj, *_in_it), std::invoke(_proj, *prev))) {
					return;
				}
			}
		}

		template <vectorizable_tag Tag>
		raze_always_inline bool operator()(Tag, sizetype aligned_size) {
			auto* ptr = std::to_address(_in_it);
			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

			raze_disable_unrolling do {
				const auto current = _proj(vx::load<Tag>(ptr));
				const auto next = _proj(vx::load<Tag>(ptr + 1));
				const auto mask = _comp(next, current);

				if (vx::any_of(mask)) {
					source_type::from_ptr(_in_it, ptr + 1 + vx::find_first_set[vx::not_null](mask));
					return false;
				}

				advance_bytes(ptr, sizeof(Tag));
			} while (ptr != aligned_end);

			source_type::from_ptr(_in_it, ptr);
			return true;
		}

		constexpr raze_always_inline iterator_type result() const {
			return _source.wrap(_in_it);
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
			requires(constexpr_sized_source<Source>)
		{
			constexpr sizetype total = Source::static_size();
			return std::integral_constant<sizetype, (total > sizeof(vector_value_type) ? total - sizeof(vector_value_type) : 0)>{};
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			const auto total = _source.size();
			return total > sizeof(vector_value_type) ? total - sizeof(vector_value_type) : sizetype(0);
		}
	};

	template <std::forward_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline It operator()(It first, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::forward_range R, class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the largest sorted subrange starting at the beginning of a range.
 *
 * 1) Examines the range `[first, last)` and finds the largest subrange `[first, it)` whose elements
 *    are sorted in non-descending order with respect to `comp` after applying `proj`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `is_sorted_until(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
 * constexpr It
 * is_sorted_until( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * is_sorted_until( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `comp`          - comparison function object returning `true` if the first argument is less than the second (defaults to `std::less<>`)
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * The upper bound of the largest sorted prefix in `[first, last)` (or `r`), i.e., an iterator to the first
 * element that is less than its predecessor, or an iterator comparing equal to `last` (or `std::ranges::end(r)`)
 * if the entire range is sorted (or has fewer than 2 elements).
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, last)` (or `std::ranges::distance(r)`):
 * at most `max(N - 1, 0)` comparisons and projections, stopping early on the first out-of-order pair.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD compares overlapping shifted vectors and locates the exact first
 *   out-of-order position via `raze::vx::find_first_set`.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/order/IsSortedUntil.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 4, 7, 3, 8, 9};
 *
 *     auto it = raze::algorithm::is_sorted_until(v);
 *     std::cout << "Sorted prefix length: " << (it - v.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * Sorted prefix length: 4
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::is_sorted`
*/
constexpr inline auto is_sorted_until = options::function_with_traits<is_sorted_until_t>;

__RAZE_ALGORITHM_NAMESPACE_END