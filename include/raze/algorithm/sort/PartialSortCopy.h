#pragma once

#include <raze/algorithm/heap/MakeHeap.h>
#include <raze/algorithm/heap/SortHeap.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct partial_sort_copy_t : Traits, dispatchable<partial_sort_copy_t<Traits>> {
	template <source Source1, source Source2, class Comp, class Proj1, class Proj2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;

		using iterator1_type = typename source1_type::iterator_type;
		using iterator2_type = typename source2_type::iterator_type;

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
		unchecked_iterator1_type _in;
		raze_no_unique_address unchecked_sentinel1_type _sent1;
		unchecked_iterator2_type _out_first;
		raze_no_unique_address unchecked_sentinel2_type _sent2;
		unchecked_iterator2_type _out_end;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Proj1 _proj1;
		raze_no_unique_address Proj2 _proj2;

		constexpr explicit kernel(Source1&& s1, Source2&& s2, Comp comp, Proj1 proj1, Proj2 proj2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _in(_source1.ubegin()),
			  _sent1(_source1.uend()),
			  _out_first(_source2.ubegin()),
			  _sent2(_source2.uend()),
			  _out_end(_out_first),
			  _comp(std::move(comp)),
			  _proj1(std::move(proj1)),
			  _proj2(std::move(proj2))
		{}

		raze_always_inline constexpr void operator()() {
			auto out_last = std::ranges::next(_out_first, _sent2);
			if (_out_first == out_last) {
				std::ranges::advance(_in, _sent1);
				_out_end = _out_first;
				return;
			}

			auto cur = _out_first;
			for (; _in != _sent1 && cur != out_last; ++_in, ++cur)
				*cur = *_in;

			const auto n = cur - _out_first;
			_out_end = cur;
			if (n == 0) return;

			if (n >= 2) {
				for (auto i = (n - 2) >> 1; ; --i) {
					detail::heap_sift_down(_out_first, n, i, _comp, _proj2);
					if (i == 0) break;
				}
			}

			for (; _in != _sent1; ++_in) {
				if (std::invoke(_comp, std::invoke(_proj1, *_in), std::invoke(_proj2, *_out_first))) {
					*_out_first = *_in;
					detail::heap_sift_down(_out_first, n, decltype(n)(0), _comp, _proj2);
				}
			}

			for (auto m = n; m > 1; --m) {
				std::ranges::iter_swap(_out_first, _out_first + (m - 1));
				detail::heap_sift_down(_out_first, m - 1, decltype(m)(0), _comp, _proj2);
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::partial_sort_copy_result<iterator1_type, iterator2_type> result() {
			return { _source1.wrap(std::move(_in)), _source2.wrap(std::move(_out_end)) };
		}
	};

	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent1,
		std::random_access_iterator OutIt, std::sentinel_for<OutIt> Sent2,
		class Comp = std::less<>, class Proj1 = std::identity, class Proj2 = std::identity>
	constexpr raze_always_inline std::ranges::partial_sort_copy_result<InIt, OutIt>
	operator()(InIt first, Sent1 last, OutIt result_first, Sent2 result_last,
		Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::indirectly_copyable<InIt, OutIt> &&
				std::sortable<OutIt, Comp, Proj2> &&
				std::indirect_strict_weak_order<Comp,
					std::projected<InIt, Proj1>, std::projected<OutIt, Proj2>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_source(std::move(result_first), std::move(result_last)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::input_range R1, std::ranges::random_access_range R2,
		class Comp = std::less<>, class Proj1 = std::identity, class Proj2 = std::identity>
	constexpr raze_always_inline
	std::ranges::partial_sort_copy_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
	operator()(R1&& r, R2&& result_r, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		requires(std::indirectly_copyable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>> &&
			std::sortable<std::ranges::iterator_t<R2>, Comp, Proj2> &&
			std::indirect_strict_weak_order<Comp,
				std::projected<std::ranges::iterator_t<R1>, Proj1>,
				std::projected<std::ranges::iterator_t<R2>, Proj2>>)
	{
		return this->dispatch(get_source(std::forward<R1>(r)), get_source(std::forward<R2>(result_r)),
			traits::fwd_fn(comp), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @ingroup raze_algo_sort
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies the smallest elements of a range into a destination range in sorted order.
 *
 * 1) Copies `K = min(distance(first, last), distance(result_first, result_last))` smallest elements
 *    from `[first, last)` into `[result_first, result_first + K)` sorted with respect to `comp`,
 *    `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r` and `result_r` as the source and destination ranges.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator InIt, std::sentinel_for<InIt> Sent1,
 *           std::random_access_iterator OutIt, std::sentinel_for<OutIt> Sent2,
 *           class Comp = std::less<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_copyable<InIt, OutIt> && std::sortable<OutIt, Comp, Proj2> &&
 *            std::indirect_strict_weak_order<Comp, std::projected<InIt, Proj1>, std::projected<OutIt, Proj2>>)
 * constexpr std::ranges::partial_sort_copy_result<InIt, OutIt>
 * partial_sort_copy( InIt first, Sent1 last, OutIt result_first, Sent2 result_last,
 *                    Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, std::ranges::random_access_range R2,
 *           class Comp = std::less<>,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(...)
 * constexpr std::ranges::partial_sort_copy_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
 * partial_sort_copy( R1&& r, R2&& result_r,
 *                    Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last`               - source input range
 * - `result_first`, `result_last` - random-access destination range
 * - `r`, `result_r`               - source and destination ranges
 * - `comp`                        - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj1`, `proj2`              - projections for input and output ranges (default to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::partial_sort_copy_result` `{in, out}` where `in == last` and `out == result_first + K`.
 *
 * ### Complexity
 * `O(N log K)` comparisons where `N = distance(first, last)` and `K = min(N, distance(result_first, result_last))`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/sort/PartialSortCopy.h>
 *
 * int main() {
 *     const std::vector<int> src{9, 4, 7, 1, 8, 2};
 *     std::vector<int> dst(3);
 *     raze::algorithm::partial_sort_copy(src, dst);
 *     for (int x : dst) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partial_sort`
 * - `raze::algorithm::sort`
 */
constexpr inline auto partial_sort_copy = options::function_with_traits<partial_sort_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END

