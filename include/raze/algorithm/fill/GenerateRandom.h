#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <functional>
#include <random>
#include <span>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class G, class It, class Sent>
concept urbg_has_generate_random_span =
	std::contiguous_iterator<It> && std::sized_sentinel_for<Sent, It> &&
	requires(G& g, It first, Sent last) {
		g.generate_random(std::span{ std::to_address(first), static_cast<std::size_t>(last - first) });
	};

template <class G, class It, class Sent>
concept urbg_has_generate_random_subrange =
	requires(G& g, It first, Sent last) {
		g.generate_random(std::ranges::subrange{ first, last });
	};

template <class D, class G, class It, class Sent>
concept dist_has_generate_random_span =
	std::contiguous_iterator<It> && std::sized_sentinel_for<Sent, It> &&
	requires(D& d, G& g, It first, Sent last) {
		d.generate_random(std::span{ std::to_address(first), static_cast<std::size_t>(last - first) }, g);
	};

template <class D, class G, class It, class Sent>
concept dist_has_generate_random_subrange =
	requires(D& d, G& g, It first, Sent last) {
		d.generate_random(std::ranges::subrange{ first, last }, g);
	};

} // namespace detail

template <class Traits>
struct generate_random_t : Traits, dispatchable<generate_random_t<Traits>> {
	template <source Source, class G>
	struct urbg_kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _it;
		raze_no_unique_address unchecked_sentinel_type _sent;
		G& _g;

		constexpr explicit urbg_kernel(Source&& src, G& g)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _g(g)
		{}

		raze_always_inline constexpr void operator()() {
			if constexpr (detail::urbg_has_generate_random_span<G, unchecked_iterator_type, unchecked_sentinel_type>) {
				auto sp = std::span{ std::to_address(_it), static_cast<std::size_t>(_sent - _it) };
				_g.generate_random(sp);
				std::ranges::advance(_it, _sent);
			}
			else if constexpr (detail::urbg_has_generate_random_subrange<G, unchecked_iterator_type, unchecked_sentinel_type>) {
				_g.generate_random(std::ranges::subrange{ _it, _sent });
				std::ranges::advance(_it, _sent);
			}
			else {
				for (; _it != _sent; ++_it)
					*_it = std::invoke(_g);
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_it));
		}
	};

	template <source Source, class G, class D>
	struct dist_kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _it;
		raze_no_unique_address unchecked_sentinel_type _sent;
		G& _g;
		D& _d;

		constexpr explicit dist_kernel(Source&& src, G& g, D& d)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _g(g),
			  _d(d)
		{}

		raze_always_inline constexpr void operator()() {
			if constexpr (detail::dist_has_generate_random_span<D, G, unchecked_iterator_type, unchecked_sentinel_type>) {
				auto sp = std::span{ std::to_address(_it), static_cast<std::size_t>(_sent - _it) };
				_d.generate_random(sp, _g);
				std::ranges::advance(_it, _sent);
			}
			else if constexpr (detail::dist_has_generate_random_subrange<D, G, unchecked_iterator_type, unchecked_sentinel_type>) {
				_d.generate_random(std::ranges::subrange{ _it, _sent }, _g);
				std::ranges::advance(_it, _sent);
			}
			else {
				for (; _it != _sent; ++_it)
					*_it = std::invoke(_d, _g);
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_it));
		}
	};

	template <source Source, class G>
	struct kernel : urbg_kernel<Source, G> {
		using urbg_kernel<Source, G>::urbg_kernel;
	};

	template <source Source, class G, class D>
	struct kernel_with_dist : dist_kernel<Source, G, D> {
		using dist_kernel<Source, G, D>::dist_kernel;
	};

	template <class G, std::input_or_output_iterator Out, std::sentinel_for<Out> Sent>
	constexpr raze_always_inline Out operator()(Out first, Sent last, G&& g) const
		requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
			std::output_iterator<Out, std::invoke_result_t<G&>>)
	{
		auto work = urbg_kernel<decltype(get_source(std::move(first), std::move(last))), std::remove_reference_t<G>>(
			get_source(std::move(first), std::move(last)), g);
		work();
		return work.result();
	}

	template <std::ranges::range R, class G>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r, G&& g) const
		requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
			std::ranges::output_range<R, std::invoke_result_t<G&>>)
	{
		auto src = get_source(std::forward<R>(r));
		auto work = urbg_kernel<decltype(src), std::remove_reference_t<G>>(std::move(src), g);
		work();
		return work.result();
	}

	template <class G, class D, std::input_or_output_iterator Out, std::sentinel_for<Out> Sent>
	constexpr raze_always_inline Out operator()(Out first, Sent last, G&& g, D&& d) const
		requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
			std::invocable<D&, G&> &&
			std::output_iterator<Out, std::invoke_result_t<D&, G&>>)
	{
		auto src = get_source(std::move(first), std::move(last));
		auto work = dist_kernel<decltype(src), std::remove_reference_t<G>, std::remove_reference_t<D>>(
			std::move(src), g, d);
		work();
		return work.result();
	}

	template <std::ranges::range R, class G, class D>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r, G&& g, D&& d) const
		requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
			std::invocable<D&, G&> &&
			std::ranges::output_range<R, std::invoke_result_t<D&, G&>>)
	{
		auto src = get_source(std::forward<R>(r));
		auto work = dist_kernel<decltype(src), std::remove_reference_t<G>, std::remove_reference_t<D>>(
			std::move(src), g, d);
		work();
		return work.result();
	}
};

/**
 * @ingroup raze_algo_fill
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Populates a range with random numbers from a uniform random bit generator or distribution (C++26 P1068).
 *
 * 1) Fills `[first, last)` with random values produced by the uniform random bit generator `g`.
 *    If `g` exposes a bulk `g.generate_random(span)` or `g.generate_random(subrange)` member function,
 *    that bulk interface is preferred; otherwise assigns `std::invoke(g)` element-wise.
 *
 * 2) Same as (1), but uses `r` as the destination range.
 *
 * 3) Fills `[first, last)` with random values drawn from distribution `d` using generator `g`.
 *    If `d` exposes a bulk `d.generate_random(span, g)` or `d.generate_random(subrange, g)` member function,
 *    that bulk interface is preferred; otherwise assigns `std::invoke(d, g)` element-wise.
 *
 * 4) Same as (3), but uses `r` as the destination range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel URBG overload
 * template< class G, std::input_or_output_iterator Out, std::sentinel_for<Out> Sent >
 *   requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
 *            std::output_iterator<Out, std::invoke_result_t<G&>>)
 * constexpr Out
 * generate_random( Out first, Sent last, G&& g );
 *
 * // (2) Range URBG overload
 * template< std::ranges::range R, class G >
 *   requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
 *            std::ranges::output_range<R, std::invoke_result_t<G&>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * generate_random( R&& r, G&& g );
 *
 * // (3) Iterator-sentinel distribution overload
 * template< class G, class D, std::input_or_output_iterator Out, std::sentinel_for<Out> Sent >
 *   requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
 *            std::invocable<D&, G&> &&
 *            std::output_iterator<Out, std::invoke_result_t<D&, G&>>)
 * constexpr Out
 * generate_random( Out first, Sent last, G&& g, D&& d );
 *
 * // (4) Range distribution overload
 * template< std::ranges::range R, class G, class D >
 *   requires(std::uniform_random_bit_generator<std::remove_reference_t<G>> &&
 *            std::invocable<D&, G&> &&
 *            std::ranges::output_range<R, std::invoke_result_t<D&, G&>>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * generate_random( R&& r, G&& g, D&& d );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the destination range to fill with random values
 * - `r`             - the destination range to fill with random values
 * - `g`             - uniform random bit generator (`std::uniform_random_bit_generator`)
 * - `d`             - random number distribution invocable with `g`
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear in `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implements the C++26 P1068 bulk random generation protocol in C++23.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <random>
 * #include <vector>
 * #include <raze/algorithm/fill/GenerateRandom.h>
 *
 * int main() {
 *     std::mt19937 rng{42};
 *     std::uniform_int_distribution<int> dist{1, 100};
 *     std::vector<int> v(8);
 *
 *     raze::algorithm::generate_random(v, rng, dist);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::generate`
 * - `raze::algorithm::shuffle`
 * - `raze::algorithm::sample`
 */
constexpr inline auto generate_random = options::function_with_traits<generate_random_t>;

__RAZE_ALGORITHM_NAMESPACE_END

