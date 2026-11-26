#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_first_of_t : Traits, dispatchable<find_first_of_t<Traits>> {
	template <source Source1, source Source2, class Predicate, class Proj1, class Proj2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;

		using iterator1_type = typename source1_type::iterator_type;
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
		unchecked_iterator1_type _it1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _first2;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Proj1 _proj1;
		raze_no_unique_address Proj2 _proj2;

		constexpr explicit kernel(Source1&& s1, Source2&& s2, Predicate pred, Proj1 proj1, Proj2 proj2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _it1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _first2(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _pred(std::move(pred)),
			  _proj1(std::move(proj1)),
			  _proj2(std::move(proj2))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it1 != _sent1; ++_it1) {
				for (auto it2 = _first2; it2 != _sent2; ++it2) {
					if (std::invoke(_pred, std::invoke(_proj1, *_it1), std::invoke(_proj2, *it2)))
						return;
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator1_type result() {
			return _source1.wrap(std::move(_it1));
		}
	};

	template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
		std::forward_iterator It2, std::sentinel_for<It2> Sent2,
		class Predicate = std::equal_to<>, class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline It1
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2,
		Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)),
			traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::input_range R1, std::ranges::forward_range R2,
		class Predicate = std::equal_to<>, class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R1>
	operator()(R1&& r1, R2&& r2, Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
			Predicate, Proj1, Proj2>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
			traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @ingroup raze_algo_find
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the first element in a range matching any element from a set of candidate values.
 *
 * 1) Searches `[first1, last1)` for the first iterator `i` such that there exists an iterator `j`
 *    in `[first2, last2)` for which `bool(std::invoke(pred, std::invoke(proj1, *i), std::invoke(proj2, *j)))`
 *    evaluates to `true`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `find_first_of(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), pred, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::forward_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>)
 * constexpr It1
 * find_first_of( It1 first1, Sent1 last1, It2 first2, Sent2 last2,
 *                Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::forward_range R2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                                       Predicate, Proj1, Proj2>)
 * constexpr std::ranges::borrowed_iterator_t<R1>
 * find_first_of( R1&& r1, R2&& r2,
 *                Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - the range of elements to examine
 * - `first2`, `last2` - the range of candidate elements to search for
 * - `r1`              - the range of elements to examine
 * - `r2`              - the range of candidate elements to search for
 * - `pred`            - binary predicate returning `true` if two elements match (defaults to `std::equal_to<>`)
 * - `proj1`           - projection applied to elements of the first range (defaults to `std::identity`)
 * - `proj2`           - projection applied to elements of the second range (defaults to `std::identity`)
 *
 * ### Return value
 * Iterator to the first element in `[first1, last1)` (or `r1`) that matches any element in `[first2, last2)` (or `r2`),
 * or an iterator equal to `last1` (or `std::ranges::end(r1)`) if no such element is found or `[first2, last2)` is empty.
 *
 * ### Complexity
 * At most `N * M` applications of `pred`, `proj1`, and `proj2`, where `N = std::ranges::distance(first1, last1)`
 * and `M = std::ranges::distance(first2, last2)`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/FindFirstOf.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 3, 4, 5};
 *     const std::vector<int> targets{9, 4, 2};
 *
 *     if (auto it = raze::algorithm::find_first_of(v, targets); it != v.end())
 *         std::cout << "Found: " << *it << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find`
 * - `raze::algorithm::search`
 */
constexpr inline auto find_first_of = options::function_with_traits<find_first_of_t>;

__RAZE_ALGORITHM_NAMESPACE_END

