#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/order/Rotate.h>
#include <raze/algorithm/partition/StablePartition.h>
#include <raze/options/Options.h>
#include <utility>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class It, class Diff, class Val, class Comp, class Proj>
constexpr It merge_lower_bound(It first, Diff len, const Val& val, Comp& comp, Proj& proj) {
	while (len > 0) {
		const Diff half = len >> 1;
		auto mid = std::ranges::next(first, half);
		if (std::invoke(comp, std::invoke(proj, *mid), val)) {
			first = std::ranges::next(mid);
			len -= half + 1;
		}
		else {
			len = half;
		}
	}
	return first;
}

template <class It, class Diff, class Val, class Comp, class Proj>
constexpr It merge_upper_bound(It first, Diff len, const Val& val, Comp& comp, Proj& proj) {
	while (len > 0) {
		const Diff half = len >> 1;
		auto mid = std::ranges::next(first, half);
		if (!std::invoke(comp, val, std::invoke(proj, *mid))) {
			first = std::ranges::next(mid);
			len -= half + 1;
		}
		else {
			len = half;
		}
	}
	return first;
}

template <class Traits, class It, class Diff, class Comp, class Proj>
constexpr void inplace_merge_without_buffer(It first, It middle, It last,
	Diff len1, Diff len2, Comp& comp, Proj& proj)
{
	if (len1 == 0 || len2 == 0) return;

	if (len1 + len2 == 2) {
		if (std::invoke(comp, std::invoke(proj, *middle), std::invoke(proj, *first)))
			std::ranges::iter_swap(first, middle);
		return;
	}

	It first_cut = first;
	It second_cut = middle;
	Diff len11 = 0;
	Diff len21 = 0;

	if (len1 > len2) {
		len11 = len1 >> 1;
		std::ranges::advance(first_cut, len11);
		second_cut = merge_lower_bound(middle, len2, std::invoke(proj, *first_cut), comp, proj);
		len21 = std::ranges::distance(middle, second_cut);
	}
	else {
		len21 = len2 >> 1;
		std::ranges::advance(second_cut, len21);
		first_cut = merge_upper_bound(first, len1, std::invoke(proj, *second_cut), comp, proj);
		len11 = std::ranges::distance(first, first_cut);
	}

	auto new_middle = rotate[options::drop_alignment<Traits>()](
		first_cut, middle, second_cut).begin();

	inplace_merge_without_buffer<Traits>(first, first_cut, new_middle, len11, len21, comp, proj);
	inplace_merge_without_buffer<Traits>(new_middle, second_cut, last,
		len1 - len11, len2 - len21, comp, proj);
}

template <class Traits, class It, class Comp, class Proj>
constexpr void inplace_merge_impl(It first, It middle, It last, Comp& comp, Proj& proj) {
	if (first == middle || middle == last) return;

	using value_type = std::iter_value_t<It>;
	const auto len1 = std::ranges::distance(first, middle);
	const auto len2 = std::ranges::distance(middle, last);

	if !consteval {
		if constexpr (std::is_move_constructible_v<value_type> && std::is_move_assignable_v<value_type>) {
			const auto min_len = (len1 < len2) ? len1 : len2;
			if (min_len >= 8) {
				temporary_buffer<value_type> buf(static_cast<std::size_t>(min_len));
				if (buf.ptr != nullptr) {
					if (len1 <= len2) {
						for (auto it = first; it != middle; ++it)
							buf.emplace_back(std::ranges::iter_move(it));

						auto* b_it = buf.ptr;
						auto* const b_end = buf.ptr + buf.constructed;
						auto r_it = middle;
						auto out = first;

						while (b_it != b_end && r_it != last) {
							if (std::invoke(comp, std::invoke(proj, *r_it), std::invoke(proj, *b_it))) {
								*out = std::ranges::iter_move(r_it);
								++r_it;
							}
							else {
								*out = std::move(*b_it);
								++b_it;
							}
							++out;
						}

						while (b_it != b_end) {
							*out = std::move(*b_it);
							++b_it;
							++out;
						}
					}
					else {
						for (auto it = middle; it != last; ++it)
							buf.emplace_back(std::ranges::iter_move(it));

						auto* b_it = buf.ptr + buf.constructed;
						auto* const b_begin = buf.ptr;
						auto l_it = middle;
						auto out = last;

						while (b_it != b_begin && l_it != first) {
							auto l_prev = std::ranges::prev(l_it);
							auto* b_prev = b_it - 1;
							--out;
							if (std::invoke(comp, std::invoke(proj, *b_prev), std::invoke(proj, *l_prev))) {
								*out = std::ranges::iter_move(l_prev);
								l_it = l_prev;
							}
							else {
								*out = std::move(*b_prev);
								b_it = b_prev;
							}
						}

						while (b_it != b_begin) {
							--b_it;
							--out;
							*out = std::move(*b_it);
						}
					}
					return;
				}
			}
		}
	}

	inplace_merge_without_buffer<Traits>(first, middle, last, len1, len2, comp, proj);
}

} // namespace detail

template <class Traits>
struct inplace_merge_t : Traits, dispatchable<inplace_merge_t<Traits>> {
	template <source Source, class Comp, class Projection>
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
		unchecked_iterator_type _middle;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _last;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, iterator_type middle, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _middle(source_type::unwrap(std::move(middle))),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_middle, _sent)),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			detail::inplace_merge_impl<Traits>(_first, _middle, _last, _comp, _proj);
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_last));
		}
	};

	template <std::bidirectional_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline It
	operator()(It first, It middle, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<It, Comp, Proj>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			std::move(middle), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::bidirectional_range R,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			std::move(middle), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_set
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Merges two consecutive sorted subranges in-place stably (C++26 P2562 `constexpr`).
 *
 * 1) Merges `[first, middle)` and `[middle, last)` in-place so that `[first, last)` is sorted
 *    with respect to `comp` and `proj`, preserving the relative order of equivalent elements.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `inplace_merge(std::ranges::begin(r), middle, std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr It
 * inplace_merge( It first, It middle, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * inplace_merge( R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the bidirectional range containing two consecutive sorted subranges
 * - `r`             - the bidirectional range
 * - `middle`        - boundary iterator separating `[first, middle)` and `[middle, last)`
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear (`N - 1` comparisons) when a temporary buffer of size `min(len1, len2)` is available;
 * falls back to `O(N log N)` in-place rotation merges during constant evaluation (`if consteval`)
 * or if memory allocation fails.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implements C++26 P2562 `constexpr` support in C++23.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/set/InplaceMerge.h>
 *
 * int main() {
 *     std::vector<int> v{1, 4, 7, 2, 3, 8};
 *     raze::algorithm::inplace_merge(v, v.begin() + 3);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::merge`
 * - `raze::algorithm::stable_sort`
 */
constexpr inline auto inplace_merge = options::function_with_traits<inplace_merge_t>;

__RAZE_ALGORITHM_NAMESPACE_END
