#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct set_difference_t : Traits, dispatchable<set_difference_t<Traits>> {
	template <source Source1, source Source2, destination Destination, class Comp, class Proj1, class Proj2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;
		using dest_type = std::remove_cvref_t<Destination>;

		using iterator1_type = typename source1_type::iterator_type;
		using out_iterator_type = typename dest_type::iterator_type;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename dest_type::unchecked_iterator_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source1_type _source1;
		raze_no_unique_address source2_type _source2;
		raze_no_unique_address dest_type _dest;
		unchecked_iterator1_type _in1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _in2;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		unchecked_out_iterator_type _out;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Proj1 _proj1;
		raze_no_unique_address Proj2 _proj2;

		constexpr explicit kernel(Source1&& s1, Source2&& s2, Destination&& dst,
			Comp comp, Proj1 proj1, Proj2 proj2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _dest(std::forward<Destination>(dst)),
			  _in1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _in2(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _out(_dest.ubegin()),
			  _comp(std::move(comp)),
			  _proj1(std::move(proj1)),
			  _proj2(std::move(proj2))
		{}

		raze_always_inline constexpr void operator()() {
			while (_in1 != _sent1 && _in2 != _sent2) {
				if (std::invoke(_comp, std::invoke(_proj1, *_in1), std::invoke(_proj2, *_in2))) {
					*_out = *_in1;
					++_in1;
					++_out;
				}
				else if (std::invoke(_comp, std::invoke(_proj2, *_in2), std::invoke(_proj1, *_in1))) {
					++_in2;
				}
				else {
					++_in1;
					++_in2;
				}
			}

			for (; _in1 != _sent1; ++_in1, ++_out)
				*_out = *_in1;
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::set_difference_result<iterator1_type, out_iterator_type> result() {
			return { _source1.wrap(std::move(_in1)), _dest.wrap(std::move(_out)) };
		}
	};

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
		std::input_iterator It2, std::sentinel_for<It2> Sent2,
		std::weakly_incrementable Out,
		class Comp = std::less<>, class Proj1 = std::identity, class Proj2 = std::identity>
	constexpr raze_always_inline std::ranges::set_difference_result<It1, Out>
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2, Out result,
		Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::mergeable<It1, It2, Out, Comp, Proj1, Proj2>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)),
			get_destination(std::move(result)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2,
		std::weakly_incrementable Out,
		class Comp = std::less<>, class Proj1 = std::identity, class Proj2 = std::identity>
	constexpr raze_always_inline
	std::ranges::set_difference_result<std::ranges::borrowed_iterator_t<R1>, Out>
	operator()(R1&& r1, R2&& r2, Out result, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		requires(std::mergeable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
			Out, Comp, Proj1, Proj2>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
			get_destination(std::move(result)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @ingroup raze_algo_set
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Computes the sorted difference of two sorted ranges.
 *
 * 1) Copies elements from `[first1, last1)` that are not found in `[first2, last2)` (accounting
 *    for multiplicity) to the destination beginning at `result` under `comp`, `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::input_iterator It2, std::sentinel_for<It2> Sent2,
 *           std::weakly_incrementable Out,
 *           class Comp = std::less<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::mergeable<It1, It2, Out, Comp, Proj1, Proj2>)
 * constexpr std::ranges::set_difference_result<It1, Out>
 * set_difference( It1 first1, Sent1 last1, It2 first2, Sent2 last2, Out result,
 *                 Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           std::weakly_incrementable Out,
 *           class Comp = std::less<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::mergeable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                           Out, Comp, Proj1, Proj2>)
 * constexpr std::ranges::set_difference_result<std::ranges::borrowed_iterator_t<R1>, Out>
 * set_difference( R1&& r1, R2&& r2, Out result,
 *                 Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - sorted minuend range
 * - `first2`, `last2` - sorted subtrahend range
 * - `r1`, `r2`        - sorted input ranges
 * - `result`          - beginning of the output range
 * - `comp`            - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj1`, `proj2`  - projections applied to each range (default to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::set_difference_result` `{in, out}`.
 *
 * ### Complexity
 * Linear: at most `2 * (N1 + N2) - 1` comparisons.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <iterator>
 * #include <vector>
 * #include <raze/algorithm/set/SetDifference.h>
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3, 4}, b{2, 4};
 *     std::vector<int> out;
 *     raze::algorithm::set_difference(a, b, std::back_inserter(out));
 *     for (int x : out) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::set_union`
 * - `raze::algorithm::set_intersection`
 * - `raze::algorithm::set_symmetric_difference`
 */
constexpr inline auto set_difference = options::function_with_traits<set_difference_t>;

__RAZE_ALGORITHM_NAMESPACE_END

