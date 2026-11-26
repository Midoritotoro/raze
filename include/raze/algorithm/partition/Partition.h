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
struct partition_t : Traits, dispatchable<partition_t<Traits>> {
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
		unchecked_iterator_type _part_point;
		unchecked_iterator_type _end_it;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _part_point(_first),
			  _end_it(_first),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			if constexpr (std::bidirectional_iterator<unchecked_iterator_type>) {
				auto last = std::ranges::next(_first, _sent);
				_end_it = last;

				auto left = _first;
				auto right = last;

				for (;;) {
					for (;;) {
						if (left == right) {
							_part_point = left;
							return;
						}
						if (!std::invoke(_pred, std::invoke(_proj, *left)))
							break;
						++left;
					}

					do {
						--right;
						if (left == right) {
							_part_point = left;
							return;
						}
					} while (!std::invoke(_pred, std::invoke(_proj, *right)));

					std::ranges::iter_swap(left, right);
					++left;
				}
			}
			else {
				auto first = _first;
				while (first != _sent && std::invoke(_pred, std::invoke(_proj, *first)))
					++first;

				if (first == _sent) {
					_part_point = first;
					_end_it = first;
					return;
				}

				auto scan = std::ranges::next(first);
				for (; scan != _sent; ++scan) {
					if (std::invoke(_pred, std::invoke(_proj, *scan))) {
						std::ranges::iter_swap(first, scan);
						++first;
					}
				}

				_part_point = first;
				_end_it = scan;
			}
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator_type> result() {
			return { _source.wrap(std::move(_part_point)), _source.wrap(std::move(_end_it)) };
		}
	};

	template <std::permutable It, std::sentinel_for<It> Sent,
		class Predicate, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It>
	operator()(It first, Sent last, Predicate pred, Proj proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::forward_range R, class Predicate, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
	operator()(R&& r, Predicate pred, Proj proj = {}) const
		requires(std::permutable<std::ranges::iterator_t<R>> &&
			std::indirect_unary_predicate<Predicate,
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
 * @brief Reorders elements so that all elements satisfying a predicate precede those that do not.
 *
 * 1) Reorders `[first, last)` such that all elements for which `std::invoke(pred, std::invoke(proj, *i))`
 *    is `true` precede all elements for which it is `false`. Relative order is not preserved.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `partition(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable It, std::sentinel_for<It> Sent,
 *           class Predicate, class Proj = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
 * constexpr std::ranges::subrange<It>
 * partition( It first, Sent last, Predicate pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R, class Predicate, class Proj = std::identity >
 *   requires(std::permutable<std::ranges::iterator_t<R>> &&
 *            std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * partition( R&& r, Predicate pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to reorder
 * - `r`             - the range of elements to reorder
 * - `pred`          - unary predicate returning `true` for elements placed in the first partition
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `{pivot, last}` spanning the second group (elements for which `pred` is `false`).
 *
 * ### Complexity
 * Linear: exactly `N` applications of `pred` and `proj`; at most `N / 2` swaps for bidirectional
 * iterators, or at most `N` swaps for forward iterators.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/partition/Partition.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5, 6};
 *     auto rest = raze::algorithm::partition(v, [](int x) { return x % 2 == 0; });
 *     for (auto it = v.begin(); it != rest.begin(); ++it) std::cout << *it << ' ';
 *     std::cout << "| ";
 *     for (int x : rest) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::stable_partition`
 * - `raze::algorithm::partition_copy`
 * - `raze::algorithm::partition_point`
 * - `raze::algorithm::is_partitioned`
 */
constexpr inline auto partition = options::function_with_traits<partition_t>;

__RAZE_ALGORITHM_NAMESPACE_END

