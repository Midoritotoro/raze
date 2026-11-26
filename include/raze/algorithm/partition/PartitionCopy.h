#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct partition_copy_t : Traits, dispatchable<partition_copy_t<Traits>> {
	template <source Source, destination DestTrue, destination DestFalse, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using dest_true_type = std::remove_cvref_t<DestTrue>;
		using dest_false_type = std::remove_cvref_t<DestFalse>;

		using iterator_type = typename source_type::iterator_type;
		using out1_type = typename dest_true_type::iterator_type;
		using out2_type = typename dest_false_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out1_type = typename dest_true_type::unchecked_iterator_type;
		using unchecked_out2_type = typename dest_false_type::unchecked_iterator_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address dest_true_type _dest_true;
		raze_no_unique_address dest_false_type _dest_false;
		unchecked_iterator_type _in;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_out1_type _out_true;
		unchecked_out2_type _out_false;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, DestTrue&& dt, DestFalse&& df, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _dest_true(std::forward<DestTrue>(dt)),
			  _dest_false(std::forward<DestFalse>(df)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out_true(_dest_true.ubegin()),
			  _out_false(_dest_false.ubegin()),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _in != _sent; ++_in) {
				if (std::invoke(_pred, std::invoke(_proj, *_in))) {
					*_out_true = *_in;
					++_out_true;
				}
				else {
					*_out_false = *_in;
					++_out_false;
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::partition_copy_result<iterator_type, out1_type, out2_type> result() {
			return {
				_source.wrap(std::move(_in)),
				_dest_true.wrap(std::move(_out_true)),
				_dest_false.wrap(std::move(_out_false))
			};
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent,
		std::weakly_incrementable Out1, std::weakly_incrementable Out2,
		class Predicate, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::partition_copy_result<It, Out1, Out2>
	operator()(It first, Sent last, Out1 out_true, Out2 out_false,
		Predicate pred, Proj proj = {}) const
			requires(std::indirectly_copyable<It, Out1> &&
				std::indirectly_copyable<It, Out2> &&
				std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(out_true)), get_destination(std::move(out_false)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R,
		std::weakly_incrementable Out1, std::weakly_incrementable Out2,
		class Predicate, class Proj = std::identity>
	constexpr raze_always_inline
	std::ranges::partition_copy_result<std::ranges::borrowed_iterator_t<R>, Out1, Out2>
	operator()(R&& r, Out1 out_true, Out2 out_false, Predicate pred, Proj proj = {}) const
		requires(std::indirectly_copyable<std::ranges::iterator_t<R>, Out1> &&
			std::indirectly_copyable<std::ranges::iterator_t<R>, Out2> &&
			std::indirect_unary_predicate<Predicate,
				std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(out_true)), get_destination(std::move(out_false)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_partition
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies elements from a range into two separate output ranges according to a predicate.
 *
 * 1) Copies each element `*i` in `[first, last)` to `out_true` if `std::invoke(pred, std::invoke(proj, *i))`
 *    is `true`, or to `out_false` otherwise, preserving relative order in both output ranges.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `partition_copy(std::ranges::begin(r), std::ranges::end(r), std::move(out_true), std::move(out_false), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out1, std::weakly_incrementable Out2,
 *           class Predicate, class Proj = std::identity >
 *   requires(std::indirectly_copyable<It, Out1> && std::indirectly_copyable<It, Out2> &&
 *            std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
 * constexpr std::ranges::partition_copy_result<It, Out1, Out2>
 * partition_copy( It first, Sent last, Out1 out_true, Out2 out_false,
 *                 Predicate pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           std::weakly_incrementable Out1, std::weakly_incrementable Out2,
 *           class Predicate, class Proj = std::identity >
 *   requires(...)
 * constexpr std::ranges::partition_copy_result<std::ranges::borrowed_iterator_t<R>, Out1, Out2>
 * partition_copy( R&& r, Out1 out_true, Out2 out_false,
 *                 Predicate pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the source range of elements
 * - `r`             - the source range of elements
 * - `out_true`      - destination range for elements satisfying `pred`
 * - `out_false`     - destination range for elements not satisfying `pred`
 * - `pred`          - unary predicate partitioning elements
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::partition_copy_result` `{in, out1, out2}` with the exhausted input iterator and
 * past-the-end iterators of both output ranges.
 *
 * ### Complexity
 * Linear: exactly `N` applications of `pred`, `proj`, and element copies.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <iterator>
 * #include <vector>
 * #include <raze/algorithm/partition/PartitionCopy.h>
 *
 * int main() {
 *     const std::vector<int> src{1, 2, 3, 4, 5, 6};
 *     std::vector<int> evens, odds;
 *     raze::algorithm::partition_copy(src, std::back_inserter(evens), std::back_inserter(odds),
 *         [](int x) { return x % 2 == 0; });
 *     std::cout << evens.size() << ' ' << odds.size() << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partition`
 * - `raze::algorithm::copy_if`
 */
constexpr inline auto partition_copy = options::function_with_traits<partition_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END

