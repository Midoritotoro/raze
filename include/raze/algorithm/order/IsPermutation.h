#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <utility>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct is_permutation_t : Traits, dispatchable<is_permutation_t<Traits>> {
	template <source Source1, source Source2, class Predicate, class Proj1, class Proj2>
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
		unchecked_iterator1_type _first1;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _first2;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Proj1 _proj1;
		raze_no_unique_address Proj2 _proj2;
		bool _result = false;

		constexpr explicit kernel(Source1&& s1, Source2&& s2, Predicate pred, Proj1 proj1, Proj2 proj2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _first1(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _first2(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _pred(std::move(pred)),
			  _proj1(std::move(proj1)),
			  _proj2(std::move(proj2))
		{}

		raze_always_inline constexpr void operator()() {
			if constexpr (std::sized_sentinel_for<unchecked_sentinel1_type, unchecked_iterator1_type> &&
				std::sized_sentinel_for<unchecked_sentinel2_type, unchecked_iterator2_type>)
			{
				if ((_sent1 - _first1) != (_sent2 - _first2)) {
					_result = false;
					return;
				}
			}

			while (_first1 != _sent1 && _first2 != _sent2) {
				if (!std::invoke(_pred, std::invoke(_proj1, *_first1), std::invoke(_proj2, *_first2)))
					break;
				++_first1;
				++_first2;
			}

			if (_first1 == _sent1 && _first2 == _sent2) {
				_result = true;
				return;
			}

			if constexpr (!(std::sized_sentinel_for<unchecked_sentinel1_type, unchecked_iterator1_type> &&
				std::sized_sentinel_for<unchecked_sentinel2_type, unchecked_iterator2_type>))
			{
				if (iter_distance(_first1, _sent1) != iter_distance(_first2, _sent2)) {
					_result = false;
					return;
				}
			}

			for (auto scan = _first1; scan != _sent1; ++scan) {
				bool seen_before = false;
				for (auto prev = _first1; prev != scan; ++prev) {
					if (std::invoke(_pred, std::invoke(_proj1, *prev), std::invoke(_proj1, *scan))) {
						seen_before = true;
						break;
					}
				}
				if (seen_before) continue;

				std::iter_difference_t<unchecked_iterator2_type> count2 = 0;
				for (auto it2 = _first2; it2 != _sent2; ++it2) {
					if (std::invoke(_pred, std::invoke(_proj1, *scan), std::invoke(_proj2, *it2)))
						++count2;
				}
				if (count2 == 0) {
					_result = false;
					return;
				}

				std::iter_difference_t<unchecked_iterator1_type> count1 = 1;
				for (auto it1 = std::ranges::next(scan); it1 != _sent1; ++it1) {
					if (std::invoke(_pred, std::invoke(_proj1, *scan), std::invoke(_proj1, *it1)))
						++count1;
				}
				if (count1 != count2) {
					_result = false;
					return;
				}
			}

			_result = true;
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}
	};

	template <std::forward_iterator It1, std::sentinel_for<It1> Sent1,
		std::forward_iterator It2, std::sentinel_for<It2> Sent2,
		class Predicate = std::equal_to<>,
		class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2,
		Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)),
			traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::forward_range R1, std::ranges::forward_range R2,
		class Predicate = std::equal_to<>,
		class Proj1 = std::identity, class Proj2 = std::identity>
	raze_nodiscard constexpr raze_always_inline bool
	operator()(R1&& r1, R2&& r2, Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
			Predicate, Proj1, Proj2>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
			traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Checks whether one range is a permutation of another range.
 *
 * 1) Returns `true` if there exists a permutation of `[first1, last1)` that makes it element-wise
 *    equivalent to `[first2, last2)` under `pred`, `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `is_permutation(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), pred, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::forward_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_comparable<It1, It2, Predicate, Proj1, Proj2>)
 * constexpr bool
 * is_permutation( It1 first1, Sent1 last1, It2 first2, Sent2 last2,
 *                 Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R1, std::ranges::forward_range R2,
 *           class Predicate = std::equal_to<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                                       Predicate, Proj1, Proj2>)
 * constexpr bool
 * is_permutation( R1&& r1, R2&& r2,
 *                 Predicate pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - the first range to compare
 * - `first2`, `last2` - the second range to compare
 * - `r1`, `r2`        - the input ranges to compare
 * - `pred`            - binary equivalence relation (defaults to `std::equal_to<>`)
 * - `proj1`, `proj2`  - projections applied to each range (default to `std::identity`)
 *
 * ### Return value
 * `true` if the two ranges have the same length and the same multiset of projected elements, `false` otherwise.
 *
 * ### Complexity
 * Linear if the ranges are already equal (or have different sizes with sized sentinels);
 * otherwise `O(N^2)` predicate applications in the worst case.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/order/IsPermutation.h>
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3, 2};
 *     const std::vector<int> b{2, 3, 1, 2};
 *     std::cout << std::boolalpha << raze::algorithm::is_permutation(a, b) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::next_permutation`
 * - `raze::algorithm::prev_permutation`
 * - `raze::algorithm::equal`
 */
constexpr inline auto is_permutation = options::function_with_traits<is_permutation_t>;

__RAZE_ALGORITHM_NAMESPACE_END

