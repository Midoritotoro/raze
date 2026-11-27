#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <utility>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct is_partitioned_t : Traits, dispatchable<is_partitioned_t<Traits>> {
	template <source Source, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _it;
		raze_no_unique_address unchecked_sentinel_type _sent;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;
		bool _result = true;

		constexpr explicit kernel(Source&& src, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it != _sent; ++_it) {
				if (!std::invoke(_pred, std::invoke(_proj, *_it)))
					break;
			}
			for (; _it != _sent; ++_it) {
				if (std::invoke(_pred, std::invoke(_proj, *_it))) {
					_result = false;
					return;
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent,
		class Predicate, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(It first, Sent last, Predicate pred, Proj proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class Predicate, class Proj = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
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
 * @brief Checks whether a range is partitioned with respect to a predicate.
 *
 * 1) Returns `true` if all elements in `[first, last)` satisfying `pred` (after applying `proj`)
 *    appear before all elements that do not satisfy `pred`. Returns `true` if the range is empty.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `is_partitioned(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Proj = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
 * constexpr bool
 * is_partitioned( It first, Sent last, Predicate pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, class Predicate, class Proj = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr bool
 * is_partitioned( R&& r, Predicate pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to examine
 * - `r`             - the range of elements to examine
 * - `pred`          - unary predicate defining the partition
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * `true` if the range is empty or partitioned by `pred`, `false` otherwise.
 *
 * ### Complexity
 * Linear: at most `N = std::ranges::distance(first, last)` applications of `pred` and `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/partition/IsPartitioned.h>
 *
 * int main() {
 *     const std::vector<int> v{2, 4, 6, 1, 3, 5};
 *     std::cout << std::boolalpha
 *               << raze::algorithm::is_partitioned(v, [](int x) { return x % 2 == 0; }) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partition`
 * - `raze::algorithm::partition_point`
 */
constexpr inline auto is_partitioned = options::function_with_traits<is_partitioned_t>;

__RAZE_ALGORITHM_NAMESPACE_END

