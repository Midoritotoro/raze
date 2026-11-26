#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/order/Reverse.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct prev_permutation_t : Traits, dispatchable<prev_permutation_t<Traits>> {
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
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _last;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;
		bool _found = false;

		constexpr explicit kernel(Source&& src, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_first, _sent)),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			if (_first == _last) {
				_found = false;
				return;
			}

			auto i = _last;
			if (_first == --i) {
				_found = false;
				return;
			}

			for (;;) {
				auto ii = i;
				--i;
				if (std::invoke(_comp, std::invoke(_proj, *ii), std::invoke(_proj, *i))) {
					auto j = _last;
					while (!std::invoke(_comp, std::invoke(_proj, *--j), std::invoke(_proj, *i))) {}
					std::ranges::iter_swap(i, j);
					reverse[options::drop_alignment<Traits>()](ii, _last);
					_found = true;
					return;
				}
				if (i == _first) {
					reverse[options::drop_alignment<Traits>()](_first, _last);
					_found = false;
					return;
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::prev_permutation_result<iterator_type> result() {
			return { _source.wrap(std::move(_last)), _found };
		}
	};

	template <std::bidirectional_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::prev_permutation_result<It>
	operator()(It first, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<It, Comp, Proj>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::bidirectional_range R,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::prev_permutation_result<std::ranges::borrowed_iterator_t<R>>
	operator()(R&& r, Comp comp = {}, Proj proj = {}) const
		requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Transforms a range into its previous lexicographically smaller permutation.
 *
 * 1) Rearranges `[first, last)` into the previous lexicographically smaller permutation with respect
 *    to `comp` and `proj`. If the range is already the lexicographically smallest permutation, reverses
 *    it into the greatest permutation and sets `found = false`.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `prev_permutation(std::ranges::begin(r), std::ranges::end(r), comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<It, Comp, Proj>)
 * constexpr std::ranges::prev_permutation_result<It>
 * prev_permutation( It first, Sent last, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R,
 *           class Comp = std::less<>, class Proj = std::identity >
 *   requires(std::sortable<std::ranges::iterator_t<R>, Comp, Proj>)
 * constexpr std::ranges::prev_permutation_result<std::ranges::borrowed_iterator_t<R>>
 * prev_permutation( R&& r, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the bidirectional range of elements to permute
 * - `r`             - the bidirectional range of elements to permute
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::prev_permutation_result` `{in, found}` where `in` compares equal to `last` and
 * `found` is `true` if a smaller permutation existed, or `false` if wrapped around to the last permutation.
 *
 * ### Complexity
 * Linear: at most `N / 2` swaps in the worst case; amortized `O(1)` over all permutations.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <string>
 * #include <raze/algorithm/order/PrevPermutation.h>
 *
 * int main() {
 *     std::string s = "cba";
 *     do {
 *         std::cout << s << '\n';
 *     } while (raze::algorithm::prev_permutation(s).found);
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::next_permutation`
 * - `raze::algorithm::is_permutation`
 */
constexpr inline auto prev_permutation = options::function_with_traits<prev_permutation_t>;

__RAZE_ALGORITHM_NAMESPACE_END
