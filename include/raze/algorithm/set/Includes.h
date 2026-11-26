#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct includes_t : Traits, dispatchable<includes_t<Traits>> {
	template <source Source1, source Source2, class Comp, class Proj1, class Proj2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source1_type _source1;
		raze_no_unique_address source2_type _source2;
		unchecked_iterator1_type _in1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _in2;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Proj1 _proj1;
		raze_no_unique_address Proj2 _proj2;
		bool _result = true;

		constexpr explicit kernel(Source1&& s1, Source2&& s2, Comp comp, Proj1 proj1, Proj2 proj2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _in1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _in2(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _comp(std::move(comp)),
			  _proj1(std::move(proj1)),
			  _proj2(std::move(proj2))
		{}

		raze_always_inline constexpr void operator()() {
			while (_in2 != _sent2) {
				if (_in1 == _sent1 || std::invoke(_comp, std::invoke(_proj2, *_in2), std::invoke(_proj1, *_in1))) {
					_result = false;
					return;
				}
				if (!std::invoke(_comp, std::invoke(_proj1, *_in1), std::invoke(_proj2, *_in2)))
					++_in2;
				++_in1;
			}
			_result = true;
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}
	};

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
		std::input_iterator It2, std::sentinel_for<It2> Sent2,
		class Comp = std::less<>, class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2,
		Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::indirect_strict_weak_order<Comp,
				std::projected<It1, Proj1>, std::projected<It2, Proj2>>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2,
		class Comp = std::less<>, class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		requires(std::indirect_strict_weak_order<Comp,
			std::projected<std::ranges::iterator_t<R1>, Proj1>,
			std::projected<std::ranges::iterator_t<R2>, Proj2>>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @ingroup raze_algo_set
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether one sorted range is a subsequence (multiset subset) of another sorted range.
 *
 * 1) Returns `true` if every element of the sorted range `[first2, last2)` is found in the sorted
 *    range `[first1, last1)` with at least the same multiplicity, under `comp`, `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::input_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Comp = std::less<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp, std::projected<It1, Proj1>, std::projected<It2, Proj2>>)
 * constexpr bool
 * includes( It1 first1, Sent1 last1, It2 first2, Sent2 last2,
 *           Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           class Comp = std::less<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirect_strict_weak_order<Comp,
 *                std::projected<std::ranges::iterator_t<R1>, Proj1>,
 *                std::projected<std::ranges::iterator_t<R2>, Proj2>>)
 * constexpr bool
 * includes( R1&& r1, R2&& r2,
 *           Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - sorted superset candidate range
 * - `first2`, `last2` - sorted subset candidate range
 * - `r1`, `r2`        - sorted input ranges
 * - `comp`            - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj1`, `proj2`  - projections applied to each range (default to `std::identity`)
 *
 * ### Return value
 * `true` if the second range is included in the first (always `true` when the second range is empty), `false` otherwise.
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
 * #include <vector>
 * #include <raze/algorithm/set/Includes.h>
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3, 4, 5}, b{2, 4};
 *     std::cout << std::boolalpha << raze::algorithm::includes(a, b) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::set_intersection`
 * - `raze::algorithm::set_difference`
 */
constexpr inline auto includes = options::function_with_traits<includes_t>;

__RAZE_ALGORITHM_NAMESPACE_END

