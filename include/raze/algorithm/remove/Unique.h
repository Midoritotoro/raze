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
struct unique_t : Traits, dispatchable<unique_t<Traits>> {
	template <source Source, class Predicate, class Projection>
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
		unchecked_iterator_type _result_begin;
		unchecked_iterator_type _result_end;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _result_begin(_first),
			  _result_end(_first),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			if (_first == _sent) return;

			auto next = std::ranges::next(_first);
			while (next != _sent && !std::invoke(_pred, std::invoke(_proj, *_first), std::invoke(_proj, *next))) {
				_first = next;
				++next;
			}

			if (next == _sent) {
				_result_begin = next;
				_result_end = next;
				return;
			}

			auto write = _first;
			auto read = next;
			while (++read != _sent) {
				if (!std::invoke(_pred, std::invoke(_proj, *write), std::invoke(_proj, *read))) {
					++write;
					*write = std::ranges::iter_move(read);
				}
			}

			_result_begin = ++write;
			_result_end = read;
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator_type> result() {
			return { _source.wrap(std::move(_result_begin)), _source.wrap(std::move(_result_end)) };
		}
	};

	template <std::permutable It, std::sentinel_for<It> Sent,
		class Predicate = std::equal_to<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It>
	operator()(It first, Sent last, Predicate pred = {}, Proj proj = {}) const
		requires(std::indirect_equivalence_relation<Predicate, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::forward_range R,
		class Predicate = std::equal_to<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
	operator()(R&& r, Predicate pred = {}, Proj proj = {}) const
		requires(std::permutable<std::ranges::iterator_t<R>> &&
			std::indirect_equivalence_relation<Predicate,
				std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_remove
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Eliminates consecutive equivalent elements from a range in-place.
 *
 * 1) Removes all but the first element from every consecutive group of equivalent elements in
 *    `[first, last)` under equivalence relation `pred` and projection `proj`, preserving the
 *    relative order of retained elements.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `unique(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable It, std::sentinel_for<It> Sent,
 *           class Predicate = std::equal_to<>, class Proj = std::identity >
 *   requires(std::indirect_equivalence_relation<Predicate, std::projected<It, Proj>>)
 * constexpr std::ranges::subrange<It>
 * unique( It first, Sent last, Predicate pred = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R,
 *           class Predicate = std::equal_to<>, class Proj = std::identity >
 *   requires(std::permutable<std::ranges::iterator_t<R>> &&
 *            std::indirect_equivalence_relation<Predicate, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * unique( R&& r, Predicate pred = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to process
 * - `r`             - the range of elements to process
 * - `pred`          - binary equivalence predicate (defaults to `std::equal_to<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `{new_last, last}` representing the tail of removed elements.
 *
 * ### Complexity
 * Linear: `max(N - 1, 0)` applications of `pred` and at most `2 * max(N - 1, 0)` projections.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/remove/Unique.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 2, 3, 3, 3, 2, 1, 1};
 *     auto tail = raze::algorithm::unique(v);
 *     v.erase(tail.begin(), tail.end());
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::unique_copy`
 * - `raze::algorithm::adjacent_find`
 * - `raze::algorithm::remove`
 */
constexpr inline auto unique = options::function_with_traits<unique_t>;

__RAZE_ALGORITHM_NAMESPACE_END

