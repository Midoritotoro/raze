#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <utility>
#include <compare>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct lexicographical_compare_three_way_t : Traits, dispatchable<lexicographical_compare_three_way_t<Traits>> {
	template <source Source1, source Source2, class Comp, class Proj1, class Proj2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;

		using result_type = std::remove_cvref_t<std::invoke_result_t<Comp&,
			std::invoke_result_t<Proj1&, std::iter_reference_t<unchecked_iterator1_type>>,
			std::invoke_result_t<Proj2&, std::iter_reference_t<unchecked_iterator2_type>>>>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source1_type _source1;
		raze_no_unique_address source2_type _source2;
		unchecked_iterator1_type _it1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _it2;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Proj1 _proj1;
		raze_no_unique_address Proj2 _proj2;
		result_type _result = std::strong_ordering::equal;

		constexpr explicit kernel(Source1&& s1, Source2&& s2, Comp comp, Proj1 proj1, Proj2 proj2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _it1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _it2(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _comp(std::move(comp)),
			  _proj1(std::move(proj1)),
			  _proj2(std::move(proj2))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it1 != _sent1 && _it2 != _sent2; ++_it1, ++_it2) {
				if (auto cmp = std::invoke(_comp, std::invoke(_proj1, *_it1), std::invoke(_proj2, *_it2)); cmp != 0) {
					_result = cmp;
					return;
				}
			}

			if (_it1 != _sent1) _result = std::strong_ordering::greater;
			else if (_it2 != _sent2) _result = std::strong_ordering::less;
			else _result = std::strong_ordering::equal;
		}

		raze_nodiscard constexpr raze_always_inline result_type result() const noexcept {
			return _result;
		}
	};

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
		std::input_iterator It2, std::sentinel_for<It2> Sent2,
		class Comp = std::compare_three_way,
		class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2,
		Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::invocable<Comp&,
				std::invoke_result_t<Proj1&, std::iter_reference_t<It1>>,
				std::invoke_result_t<Proj2&, std::iter_reference_t<It2>>>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2,
		class Comp = std::compare_three_way,
		class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline auto
	operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		requires(std::invocable<Comp&,
			std::invoke_result_t<Proj1&, std::ranges::range_reference_t<R1>>,
			std::invoke_result_t<Proj2&, std::ranges::range_reference_t<R2>>>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @ingroup raze_algo_query
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Performs lexicographical three-way comparison of two ranges.
 *
 * 1) Compares elements of `[first1, last1)` and `[first2, last2)` pairwise using the three-way
 *    comparator `comp` after applying projections `proj1` and `proj2`, returning the first non-zero
 *    comparison result or an ordering based on remaining range lengths.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `lexicographical_compare_three_way(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), comp, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::input_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Comp = std::compare_three_way,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 * constexpr auto
 * lexicographical_compare_three_way( It1 first1, Sent1 last1, It2 first2, Sent2 last2,
 *                                    Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           class Comp = std::compare_three_way,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 * constexpr auto
 * lexicographical_compare_three_way( R1&& r1, R2&& r2,
 *                                    Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - the first range of elements to compare
 * - `first2`, `last2` - the second range of elements to compare
 * - `r1`, `r2`        - the input ranges to compare
 * - `comp`            - three-way comparison function object (defaults to `std::compare_three_way`)
 * - `proj1`, `proj2`  - projections applied to elements of each range (default to `std::identity`)
 *
 * ### Return value
 * The first non-equivalent comparison category value returned by `comp`, or the length comparison
 * converted to `comp`'s return type if one range is a prefix of the other.
 *
 * ### Complexity
 * At most `min(N1, N2)` applications of `comp`, `proj1`, and `proj2`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/LexicographicalCompareThreeWay.h>
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3};
 *     const std::vector<int> b{1, 2, 4};
 *     std::cout << (raze::algorithm::lexicographical_compare_three_way(a, b) < 0) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::lexicographical_compare`
 */
constexpr inline auto lexicographical_compare_three_way =
	options::function_with_traits<lexicographical_compare_three_way_t>;

__RAZE_ALGORITHM_NAMESPACE_END

