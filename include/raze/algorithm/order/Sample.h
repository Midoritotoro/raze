#pragma once

#include <raze/algorithm/order/Shuffle.h>
#include <src/raze/algorithm/Destination.h>
#include <algorithm>
#include <iterator>
#include <random>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct sample_t : Traits, dispatchable<sample_t<Traits>> {
	template <source Source, destination Destination, class Gen>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using out_iterator_type = typename destination_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;
		using difference_type = std::iter_difference_t<unchecked_iterator_type>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address destination_type _dest;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_out_iterator_type _out;
		difference_type _n;
		Gen& _g;

		constexpr explicit kernel(Source&& src, Destination&& dst, difference_type n, Gen& g)
			: _source(std::forward<Source>(src)),
			  _dest(std::forward<Destination>(dst)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _out(_dest.ubegin()),
			  _n(n),
			  _g(g)
		{}

		raze_always_inline constexpr void operator()() {
			if (_n <= 0) return;

			if constexpr (std::forward_iterator<unchecked_iterator_type>) {
				difference_type unsampled = iter_distance(_first, _sent);
				difference_type remaining = (_n < unsampled) ? _n : unsampled;

				while (remaining > 0) {
					const difference_type r = detail::uniform_bounded_index<difference_type>(_g, unsampled);
					if (r < remaining) {
						*_out = *_first;
						++_out;
						--remaining;
					}
					++_first;
					--unsampled;
				}
			}
			else {
				difference_type count = 0;
				while (_first != _sent && count < _n) {
					_out[count] = *_first;
					++_first;
					++count;
				}

				difference_type seen = count;
				while (_first != _sent) {
					++seen;
					const difference_type idx = detail::uniform_bounded_index<difference_type>(_g, seen);
					if (idx < _n)
						_out[idx] = *_first;
					++_first;
				}

				_out += count;
			}
		}

		raze_nodiscard constexpr raze_always_inline out_iterator_type result() {
			return _dest.wrap(std::move(_out));
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out, class Gen>
	constexpr raze_always_inline Out
	operator()(It first, Sent last, Out out, std::iter_difference_t<It> n, Gen&& g) const
		requires((std::forward_iterator<It> || std::random_access_iterator<Out>) &&
			std::indirectly_copyable<It, Out> &&
			std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(out)), n, g);
	}

	template <std::ranges::input_range R, std::weakly_incrementable Out, class Gen>
	constexpr raze_always_inline Out
	operator()(R&& r, Out out, std::ranges::range_difference_t<R> n, Gen&& g) const
		requires((std::ranges::forward_range<R> || std::random_access_iterator<Out>) &&
			std::indirectly_copyable<std::ranges::iterator_t<R>, Out> &&
			std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(out)), n, g);
	}
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Selects `n` elements uniformly at random without replacement from a range.
 *
 * 1) Selects `min(n, std::ranges::distance(first, last))` elements from `[first, last)` without
 *    replacement and writes them to the output range beginning at `out`, using generator `g`.
 *    Uses selection sampling (preserving relative order) when `It` models `std::forward_iterator`,
 *    and reservoir sampling otherwise.
 *
 * 2) Same as (1), but uses `r` as the population range, as if by:
 *    `sample(std::ranges::begin(r), std::ranges::end(r), std::move(out), n, std::forward<Gen>(g))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable Out, class Gen >
 *   requires((std::forward_iterator<It> || std::random_access_iterator<Out>) &&
 *            std::indirectly_copyable<It, Out> &&
 *            std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
 * constexpr Out
 * sample( It first, Sent last, Out out, std::iter_difference_t<It> n, Gen&& g );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable Out, class Gen >
 *   requires((std::ranges::forward_range<R> || std::random_access_iterator<Out>) &&
 *            std::indirectly_copyable<std::ranges::iterator_t<R>, Out> &&
 *            std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
 * constexpr Out
 * sample( R&& r, Out out, std::ranges::range_difference_t<R> n, Gen&& g );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the population range to sample from
 * - `r`             - the population range to sample from
 * - `out`           - the beginning of the destination range
 * - `n`             - maximum number of elements to sample
 * - `g`             - uniform random bit generator (`std::uniform_random_bit_generator`)
 *
 * ### Return value
 * An iterator pointing past the last written sample element in the destination range.
 *
 * ### Complexity
 * Linear in `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <iterator>
 * #include <random>
 * #include <vector>
 * #include <raze/algorithm/order/Sample.h>
 *
 * int main() {
 *     const std::vector<int> pop{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
 *     std::vector<int> out;
 *     std::mt19937 rng{42};
 *
 *     raze::algorithm::sample(pop, std::back_inserter(out), 4, rng);
 *     for (int x : out) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::shuffle`
 * - `raze::algorithm::generate_random`
 */
constexpr inline auto sample = options::function_with_traits<sample_t>;

__RAZE_ALGORITHM_NAMESPACE_END

