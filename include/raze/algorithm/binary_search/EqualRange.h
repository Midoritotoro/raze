#pragma once

#include <raze/algorithm/binary_search/LowerBound.h>
#include <raze/algorithm/binary_search/UpperBound.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct equal_range_t : Traits, dispatchable<equal_range_t<Traits>> {
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
		unchecked_iterator_type _res_begin;
		unchecked_iterator_type _res_end;
		const T& _value;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, const T& value, Comp comp, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _res_begin(_first),
			  _res_end(_first),
			  _value(value),
			  _comp(std::move(comp)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			auto len = std::ranges::distance(_first, _sent);
			while (len > 0) {
				const auto half = len >> 1;
				auto mid = std::ranges::next(_first, half);

				if (std::invoke(_comp, std::invoke(_proj, *mid), _value)) {
					_first = std::ranges::next(mid);
					len -= half + 1;
				}
				else if (std::invoke(_comp, _value, std::invoke(_proj, *mid))) {
					len = half;
				}
				else {
					auto left = _first;
					auto left_len = half;
					while (left_len > 0) {
						const auto lh = left_len >> 1;
						auto lm = std::ranges::next(left, lh);
						if (std::invoke(_comp, std::invoke(_proj, *lm), _value)) {
							left = std::ranges::next(lm);
							left_len -= lh + 1;
						}
						else {
							left_len = lh;
						}
					}

					auto right = std::ranges::next(mid);
					auto right_len = len - (half + 1);
					while (right_len > 0) {
						const auto rh = right_len >> 1;
						auto rm = std::ranges::next(right, rh);
						if (!std::invoke(_comp, _value, std::invoke(_proj, *rm))) {
							right = std::ranges::next(rm);
							right_len -= rh + 1;
						}
						else {
							right_len = rh;
						}
					}

					_res_begin = left;
					_res_end = right;
					return;
				}
			}

			_res_begin = _first;
			_res_end = _first;
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator_type> result() {
			return { _source.wrap(std::move(_res_begin)), _source.wrap(std::move(_res_end)) };
		}
	};

	template <std::forward_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity,
		class T = projected_value_t<It, Proj>>
	raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
	operator()(It first, Sent last, const T& value, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			value, traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::forward_range R,
		class Comp = std::less<>, class Proj = std::identity,
		class T = projected_value_t<std::ranges::iterator_t<R>, Proj>>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
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
 * @brief Finds the subrange of all elements equivalent to a given value in a sorted range.
 *
 * 1) Returns `{lower_bound(first, last, value, comp, proj), upper_bound(first, last, value, comp, proj)}`
 *    using a combined binary search.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `equal_range(std::ranges::begin(r), std::ranges::end(r), value, comp, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It, std::sentinel_for<It> Sent,
 *           class Comp = std::less<>, class Proj = std::identity,
 *           class T = projected_value_t<It, Proj> >
 *   requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<It, Proj>>)
 * constexpr std::ranges::subrange<It>
 * equal_range( It first, Sent last, const T& value, Comp comp = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class Comp = std::less<>, class Proj = std::identity,
 *           class T = projected_value_t<std::ranges::iterator_t<R>, Proj> >
 *   requires(std::indirect_strict_weak_order<Comp, const T*, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * equal_range( R&& r, const T& value, Comp comp = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the sorted range to search
 * - `r`             - the sorted range to search
 * - `value`         - value to compare elements against (supports braced-init lists via C++26 P2248)
 * - `comp`          - strict weak ordering comparator (defaults to `std::less<>`)
 * - `proj`          - projection applied to range elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` spanning all elements equivalent to `value`.
 *
 * ### Complexity
 * Logarithmic: at most `2 * log2(N) + 1` comparisons.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/binary_search/EqualRange.h>
 *
 * int main() {
 *     const std::vector<int> v{1, 2, 4, 4, 4, 7};
 *     auto eq = raze::algorithm::equal_range(v, 4);
 *     std::cout << "Count of 4: " << eq.size() << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::lower_bound`
 * - `raze::algorithm::upper_bound`
 * - `raze::algorithm::binary_search`
 */
constexpr inline auto equal_range = options::function_with_traits<equal_range_t>;

__RAZE_ALGORITHM_NAMESPACE_END

