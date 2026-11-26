#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <functional>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class In, class Out>
concept unique_copy_readable_output =
	std::input_iterator<Out> &&
	std::same_as<std::iter_value_t<In>, std::iter_value_t<Out>>;

} // namespace detail

template <class Traits>
struct unique_copy_t : Traits, dispatchable<unique_copy_t<Traits>> {
	template <source Source, destination Destination, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using out_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address destination_type _dest;
		unchecked_iterator_type _in;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_out_iterator_type _out;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Destination&& dst, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _in(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			if (_in == _sent) return;

			if constexpr (std::forward_iterator<unchecked_iterator_type>) {
				auto prev = _in;
				*_out = *_in;
				++_out;
				while (++_in != _sent) {
					if (!std::invoke(_pred, std::invoke(_proj, *prev), std::invoke(_proj, *_in))) {
						prev = _in;
						*_out = *_in;
						++_out;
					}
				}
			}
			else if constexpr (detail::unique_copy_readable_output<unchecked_iterator_type, unchecked_out_iterator_type>)
			{
				*_out = *_in;
				while (++_in != _sent) {
					if (!std::invoke(_pred, std::invoke(_proj, *_out), std::invoke(_proj, *_in))) {
						++_out;
						*_out = *_in;
					}
				}
				++_out;
			}
			else {
				std::iter_value_t<unchecked_iterator_type> value = *_in;
				*_out = value;
				++_out;
				while (++_in != _sent) {
					if (!std::invoke(_pred, std::invoke(_proj, value), std::invoke(_proj, *_in))) {
						value = *_in;
						*_out = value;
						++_out;
					}
				}
			}
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::unique_copy_result<iterator_type, out_iterator_type> result() {
			return { _source.wrap(std::move(_in)), _dest.wrap(std::move(_out)) };
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
		class Predicate = std::equal_to<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unique_copy_result<It, Out>
	operator()(It first, Sent last, Out result, Predicate pred = {}, Proj proj = {}) const
		requires(std::indirectly_copyable<It, Out> &&
			std::indirect_equivalence_relation<Predicate, std::projected<It, Proj>> &&
			(std::forward_iterator<It> ||
			 detail::unique_copy_readable_output<It, Out> ||
			 std::indirectly_copyable_storable<It, Out>))
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(result)), traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out,
		class Predicate = std::equal_to<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unique_copy_result<std::ranges::borrowed_iterator_t<R>, Out>
	operator()(R&& r, Out result, Predicate pred = {}, Proj proj = {}) const
		requires(std::indirectly_copyable<std::ranges::iterator_t<R>, Out> &&
			std::indirect_equivalence_relation<Predicate,
				std::projected<std::ranges::iterator_t<R>, Proj>> &&
			(std::forward_iterator<std::ranges::iterator_t<R>> ||
			 detail::unique_copy_readable_output<std::ranges::iterator_t<R>, Out> ||
			 std::indirectly_copyable_storable<std::ranges::iterator_t<R>, Out>))
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(result)), traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_remove
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies elements from a range while omitting consecutive equivalent elements.
 *
 * 1) Copies the first element from each consecutive group of equivalent elements in `[first, last)`
 *    into the destination range beginning at `result`, comparing elements via `pred` and `proj`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `unique_copy(std::ranges::begin(r), std::ranges::end(r), std::move(result), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out,
 *           class Predicate = std::equal_to<>, class Proj = std::identity >
 *   requires(std::indirectly_copyable<It, Out> &&
 *            std::indirect_equivalence_relation<Predicate, std::projected<It, Proj>> &&
 *            (std::forward_iterator<It> ||
 *             (std::input_iterator<Out> && std::same_as<std::iter_value_t<It>, std::iter_value_t<Out>>) ||
 *             std::indirectly_copyable_storable<It, Out>))
 * constexpr std::ranges::unique_copy_result<It, Out>
 * unique_copy( It first, Sent last, Out result, Predicate pred = {}, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable Out,
 *           class Predicate = std::equal_to<>, class Proj = std::identity >
 *   requires(...)
 * constexpr std::ranges::unique_copy_result<std::ranges::borrowed_iterator_t<R>, Out>
 * unique_copy( R&& r, Out result, Predicate pred = {}, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the source range of elements to examine
 * - `r`             - the source range of elements to examine
 * - `result`        - the beginning of the destination range
 * - `pred`          - binary equivalence predicate (defaults to `std::equal_to<>`)
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::unique_copy_result` `{in, out}` holding the exhausted input iterator and
 * past-the-end output iterator.
 *
 * ### Complexity
 * Linear: `max(N - 1, 0)` applications of `pred` and at most `N` assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <iterator>
 * #include <vector>
 * #include <raze/algorithm/remove/UniqueCopy.h>
 *
 * int main() {
 *     const std::vector<int> src{1, 1, 2, 2, 2, 3, 1};
 *     std::vector<int> dst;
 *     raze::algorithm::unique_copy(src, std::back_inserter(dst));
 *     for (int x : dst) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::unique`
 * - `raze::algorithm::copy`
 */
constexpr inline auto unique_copy = options::function_with_traits<unique_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END

