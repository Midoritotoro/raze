#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/ProjectedValue.h>
#include <raze/options/Options.h>
#include <utility>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct lower_bound_t : Traits, dispatchable<lower_bound_t<Traits>> {
	template <source Source, class T, class Comp, class Projection>
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
		const T& _value;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, const T& value, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _value(value),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			auto len = iter_distance(_first, _sent);
			while (len > 0) {
				const auto half = len >> 1;
				auto mid = std::ranges::next(_first, half);
				if (std::invoke(_comp, std::invoke(_proj, *mid), _value)) {
					_first = std::ranges::next(mid);
					len -= half + 1;
				}
				else {
					len = half;
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_first));
		}
	};

	template <std::forward_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity,
		class T = projected_value_t<It, Proj>>
	raze_nodiscard constexpr raze_always_inline It
	operator()(It first, Sent last, const T& value, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			value, traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::forward_range R,
		class Comp = std::less<>, class Proj = std::identity,
		class T = projected_value_t<std::ranges::iterator_t<R>, Proj>>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, const T*,
			std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			value, traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_binary_search
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds the first element in a partitioned/sorted range that is not less than a given value.
 *
 * 1) Performs a binary search on `[first, last)` to locate the first iterator `i` such that
 *    `bool(std::invoke(comp, std::invoke(proj, *i), value))` is `false`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `lower_bound(std::ranges::begin(r), std::ranges::end(r), value, comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity,
 *           class T = projected_value_t<It, Proj> >
 *   requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<It, Proj>>)
 * constexpr It
 * lower_bound( It first, Sent last, const T& value, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class Comp = std::less<>, class Proj = std::identity,
 *           class T = projected_value_t<std::ranges::iterator_t<R>, Proj> >
 *   requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * lower_bound( R&& r, const T& value, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the sorted (or partitioned with respect to `value`) range to search
 * - `r`             - the sorted range to search
 * - `value`         - value to compare elements against (supports braced-init lists via C++26 P2248)
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to range elements (defaults to `std::identity`)
 *
 * ### Return value
 * Iterator to the first element not less than `value`, or an iterator equal to `last` if all elements are less.
 *
 * ### Complexity
 * Logarithmic: at most `log2(N) + 1` comparisons (and `O(N)` iterator increments for non-random-access iterators).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/binary_search/LowerBound.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 4, 4, 5, 7};
 *     auto it = raze::algorithm::lower_bound(v, 4);
 *     std::cout << "Index: " << (it - v.begin()) << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::upper_bound`
 * - `raze::algorithm::equal_range`
 * - `raze::algorithm::binary_search`
 */
constexpr inline auto lower_bound = options::function_with_traits<lower_bound_t>;

__RAZE_ALGORITHM_NAMESPACE_END

