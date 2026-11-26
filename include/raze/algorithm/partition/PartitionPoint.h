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
struct partition_point_t : Traits, dispatchable<partition_point_t<Traits>> {
	template <source Source, class Predicate, class Projection>
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
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			auto len = std::ranges::distance(_first, _sent);
			while (len > 0) {
				const auto half = len >> 1;
				auto mid = std::ranges::next(_first, half);
				if (std::invoke(_pred, std::invoke(_proj, *mid))) {
					_first = std::ranges::next(mid);
					len -= half + 1;
				}
				else {
					len = half;
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_first));
		}
	};

	template <std::forward_iterator It, std::sentinel_for<It> Sent,
		class Predicate, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline It
	operator()(It first, Sent last, Predicate pred, Proj proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::forward_range R, class Predicate, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, Predicate pred, Proj proj = {}) const
		requires(std::indirect_unary_predicate<Predicate,
			std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_partition
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Locates the first element in a partitioned range that does not satisfy a predicate.
 *
 * 1) Performs a binary search on the partitioned range `[first, last)` to find the first iterator `i`
 *    for which `bool(std::invoke(pred, std::invoke(proj, *i)))` is `false`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `partition_point(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Proj = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
 * constexpr It
 * partition_point( It first, Sent last, Predicate pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R, class Predicate, class Proj = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * partition_point( R&& r, Predicate pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the partitioned range of elements to search
 * - `r`             - the partitioned range of elements to search
 * - `pred`          - unary predicate by which the range is partitioned
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * Iterator to the first element not satisfying `pred`, or an iterator equal to `last` if all elements satisfy `pred`.
 *
 * ### Complexity
 * Logarithmic in `N = std::ranges::distance(first, last)`: at most `log2(N) + 1` predicate applications
 * (with `O(N)` iterator increments for non-random-access iterators).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/partition/PartitionPoint.h>
 *
 * int main() {
 *     const std::vector<int> v{2, 4, 6, 1, 3, 5};
 *     auto it = raze::algorithm::partition_point(v, [](int x) { return x % 2 == 0; });
 *     std::cout << "First odd: " << *it << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partition`
 * - `raze::algorithm::is_partitioned`
 * - `raze::algorithm::lower_bound`
 */
constexpr inline auto partition_point = options::function_with_traits<partition_point_t>;

__RAZE_ALGORITHM_NAMESPACE_END

