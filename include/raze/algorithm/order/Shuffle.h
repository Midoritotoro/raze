#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <algorithm>
#include <iterator>
#include <random>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class Diff, class Gen>
constexpr Diff uniform_bounded_index(Gen& g, Diff bound) {
	if !consteval {
		std::uniform_int_distribution<Diff> dist(Diff(0), bound - 1);
		return dist(g);
	}
	else {
		using U = std::make_unsigned_t<Diff>;
		using GRes = std::remove_cvref_t<decltype(g())>;
		using Wide = std::conditional_t<(sizeof(U) >= sizeof(GRes)), U, GRes>;

		const Wide ub = static_cast<Wide>(bound);
		constexpr Wide gmin = static_cast<Wide>(std::remove_cvref_t<Gen>::min());
		constexpr Wide gmax = static_cast<Wide>(std::remove_cvref_t<Gen>::max());
		const Wide grange = gmax - gmin;

		if (ub <= 1) return Diff(0);

		const Wide limit = (grange == static_cast<Wide>(-1))
			? (static_cast<Wide>(-1) - (static_cast<Wide>(-1) % ub + 1) % ub)
			: (grange - ((grange + 1) % ub));

		for (;;) {
			const Wide r = static_cast<Wide>(g()) - gmin;
			if (r <= limit)
				return static_cast<Diff>(r % ub);
		}
	}
}

} // namespace detail

template <class Traits>
struct shuffle_t : Traits, dispatchable<shuffle_t<Traits>> {
	template <source Source, class Gen>
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
		unchecked_iterator_type _last;
		Gen& _g;

		constexpr explicit kernel(Source&& src, Gen& g)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _last(std::ranges::next(_first, _sent)),
			  _g(g)
		{}

		raze_always_inline constexpr void operator()() {
			const difference_type n = _last - _first;
			if (n <= 1) return;

			for (difference_type i = n - 1; i > 0; --i) {
				const difference_type j = detail::uniform_bounded_index<difference_type>(_g, i + 1);
				if (i != j)
					std::ranges::iter_swap(_first + i, _first + j);
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_last));
		}
	};

	template <std::random_access_iterator It, std::sentinel_for<It> Sent, class Gen>
	constexpr raze_always_inline It operator()(It first, Sent last, Gen&& g) const
		requires(std::permutable<It> && std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), g);
	}

	template <std::ranges::random_access_range R, class Gen>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r, Gen&& g) const
		requires(std::permutable<std::ranges::iterator_t<R>> &&
			std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), g);
	}
};

/**
 * @ingroup raze_algo_order
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Randomly reorders elements in a range with uniform probability.
 *
 * 1) Reorders the elements in `[first, last)` using Fisher-Yates shuffling driven by the
 *    uniform random bit generator `g`, so that every permutation has equal probability.
 *
 * 2) Same as (1), but uses `r` as the range to shuffle, as if by:
 *    `shuffle(std::ranges::begin(r), std::ranges::end(r), std::forward<Gen>(g))`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::random_access_iterator It, std::sentinel_for<It> Sent, class Gen >
 *   requires(std::permutable<It> && std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
 * constexpr It
 * shuffle( It first, Sent last, Gen&& g );
 *
 * // (2) Range overload
 * template< std::ranges::random_access_range R, class Gen >
 *   requires(std::permutable<std::ranges::iterator_t<R>> &&
 *            std::uniform_random_bit_generator<std::remove_reference_t<Gen>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * shuffle( R&& r, Gen&& g );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the random-access range of elements to shuffle
 * - `r`             - the random-access range of elements to shuffle
 * - `g`             - uniform random bit generator (`std::uniform_random_bit_generator`)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear: `max(N - 1, 0)` swaps and random index draws.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <random>
 * #include <vector>
 * #include <raze/algorithm/order/Shuffle.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5};
 *     std::mt19937 rng{42};
 *     raze::algorithm::shuffle(v, rng);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::sample`
 * - `raze::algorithm::generate_random`
 */
constexpr inline auto shuffle = options::function_with_traits<shuffle_t>;

__RAZE_ALGORITHM_NAMESPACE_END

