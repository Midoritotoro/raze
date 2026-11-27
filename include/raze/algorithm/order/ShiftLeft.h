#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <utility>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct shift_left_t : Traits, dispatchable<shift_left_t<Traits>> {
	template <source Source>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using difference_type = std::iter_difference_t<unchecked_iterator_type>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		difference_type _n;
		unchecked_iterator_type _res_begin;
		unchecked_iterator_type _res_end;

		constexpr explicit kernel(Source&& src, difference_type n)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _n(n),
			  _res_begin(_first),
			  _res_end(_first)
		{}

		raze_always_inline constexpr void operator()() {
			if (_n <= 0) {
				_res_begin = _first;
				_res_end = std::ranges::next(_first, _sent);
				return;
			}

			auto mid = _first;
			if (std::ranges::advance(mid, _n, _sent) != 0) {
				_res_begin = _first;
				_res_end = _first;
				return;
			}

			auto out = _first;
			for (; mid != _sent; ++mid, ++out)
				*out = std::ranges::iter_move(mid);

			_res_begin = _first;
			_res_end = out;
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator_type> result() {
			return { _source.wrap(std::move(_res_begin)), _source.wrap(std::move(_res_end)) };
		}
	};

	template <std::permutable It, std::sentinel_for<It> Sent>
	constexpr raze_always_inline std::ranges::subrange<It>
	operator()(It first, Sent last, std::iter_difference_t<It> n) const {
		return this->dispatch(get_source(std::move(first), std::move(last)), n);
	}

	template <std::ranges::forward_range R>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
	operator()(R&& r, std::ranges::range_difference_t<R> n) const
		requires(std::permutable<std::ranges::iterator_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), n);
	}
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Shifts elements in a range left by `n` positions (C++23).
 *
 * 1) Moves each element at position `i >= n` in `[first, last)` to position `i - n` while
 *    preserving relative order. If `n <= 0`, does nothing and returns `{first, last}`.
 *    If `n >= std::ranges::distance(first, last)`, does nothing and returns `{first, first}`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `shift_left(std::ranges::begin(r), std::ranges::end(r), n)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable It, std::sentinel_for<It> Sent >
 * constexpr std::ranges::subrange<It>
 * shift_left( It first, Sent last, std::iter_difference_t<It> n );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R >
 *   requires(std::permutable<std::ranges::iterator_t<R>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * shift_left( R&& r, std::ranges::range_difference_t<R> n );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to shift
 * - `r`             - the range of elements to shift
 * - `n`             - the number of positions to shift left
 *
 * ### Return value
 * A `std::ranges::subrange` `{first, new_end}` spanning the shifted elements.
 *
 * ### Complexity
 * Linear: at most `max(N - n, 0)` move assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/order/ShiftLeft.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5};
 *     auto res = raze::algorithm::shift_left(v, 2);
 *     for (int x : res) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::shift_right`
 * - `raze::algorithm::rotate`
 * - `raze::algorithm::move`
 */
constexpr inline auto shift_left = options::function_with_traits<shift_left_t>;

__RAZE_ALGORITHM_NAMESPACE_END

