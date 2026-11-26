#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct uninitialized_move_t : Traits, dispatchable<uninitialized_move_t<Traits>> {
	template <source Source1, source Source2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;

		using iterator1_type = typename source1_type::iterator_type;
		using iterator2_type = typename source2_type::iterator_type;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source1_type _source1;
		raze_no_unique_address source2_type _source2;
		unchecked_iterator1_type _in;
		raze_no_unique_address unchecked_sentinel1_type _in_sent;
		unchecked_iterator2_type _out_first;
		raze_no_unique_address unchecked_sentinel2_type _out_sent;
		unchecked_iterator2_type _out;

		constexpr explicit kernel(Source1&& s1, Source2&& s2)
			: _source1(std::forward<Source1>(s1)),
			  _source2(std::forward<Source2>(s2)),
			  _in(_source1.ubegin()),
			  _in_sent(_source1.uend()),
			  _out_first(_source2.ubegin()),
			  _out_sent(_source2.uend()),
			  _out(_out_first)
		{}

		raze_always_inline constexpr void operator()() {
			detail::uninit_destroy_guard<unchecked_iterator2_type> guard{ _out_first, _out };
			for (; _in != _in_sent && _out != _out_sent; ++_in, ++_out)
				std::construct_at(std::addressof(*_out), std::ranges::iter_move(_in));
			guard.active = false;
		}

		raze_nodiscard constexpr raze_always_inline
		std::ranges::uninitialized_move_result<iterator1_type, iterator2_type> result() {
			return { _source1.wrap(std::move(_in)), _source2.wrap(std::move(_out)) };
		}
	};

	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent1,
		detail::nothrow_forward_iterator OutIt, detail::nothrow_sentinel_for<OutIt> Sent2>
	constexpr raze_always_inline std::ranges::uninitialized_move_result<InIt, OutIt>
	operator()(InIt ifirst, Sent1 ilast, OutIt ofirst, Sent2 olast) const
		requires(std::constructible_from<std::iter_value_t<OutIt>, std::iter_rvalue_reference_t<InIt>>)
	{
		return this->dispatch(get_source(std::move(ifirst), std::move(ilast)),
			get_source(std::move(ofirst), std::move(olast)));
	}

	template <std::ranges::input_range R1, detail::nothrow_forward_range R2>
	constexpr raze_always_inline
	std::ranges::uninitialized_move_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
	operator()(R1&& irange, R2&& orange) const
		requires(std::constructible_from<std::ranges::range_value_t<R2>, std::ranges::range_rvalue_reference_t<R1>>)
	{
		return this->dispatch(get_source(std::forward<R1>(irange)), get_source(std::forward<R2>(orange)));
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Move-constructs objects from an input range into an uninitialized destination range.
 *
 * 1) Moves elements from `[ifirst, ilast)` into uninitialized storage `[ofirst, olast)` via
 *    `std::construct_at` and `std::ranges::iter_move`. Destroys all constructed destination objects
 *    if an exception is thrown.
 *
 * 2) Same as (1), using ranges `irange` and `orange`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator InIt, std::sentinel_for<InIt> Sent1,
 *           nothrow_forward_iterator OutIt, nothrow_sentinel_for<OutIt> Sent2 >
 *   requires(std::constructible_from<std::iter_value_t<OutIt>, std::iter_rvalue_reference_t<InIt>>)
 * constexpr std::ranges::uninitialized_move_result<InIt, OutIt>
 * uninitialized_move( InIt ifirst, Sent1 ilast, OutIt ofirst, Sent2 olast );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R1, nothrow_forward_range R2 >
 *   requires(std::constructible_from<std::ranges::range_value_t<R2>, std::ranges::range_rvalue_reference_t<R1>>)
 * constexpr std::ranges::uninitialized_move_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
 * uninitialized_move( R1&& irange, R2&& orange );
 * ```
 *
 * ### Parameters
 * - `ifirst`, `ilast`, `irange` - source range of elements
 * - `ofirst`, `olast`, `orange` - uninitialized destination range
 *
 * ### Return value
 * A `std::ranges::uninitialized_move_result` `{in, out}`.
 *
 * ### Complexity
 * Linear: `min(N1, N2)` move constructions.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::uninitialized_move_n`
 * - `raze::algorithm::uninitialized_copy`
 * - `raze::algorithm::destroy`
 */
constexpr inline auto uninitialized_move = options::function_with_traits<uninitialized_move_t>;

__RAZE_ALGORITHM_NAMESPACE_END

