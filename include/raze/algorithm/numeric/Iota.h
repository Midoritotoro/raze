#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/options/Options.h>
#include <iterator>
#include <numeric>
#include <utility>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct iota_t : Traits, dispatchable<iota_t<Traits>> {
	template <source Source, class T>
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
		unchecked_iterator_type _it;
		raze_no_unique_address unchecked_sentinel_type _sent;
		T _value;

		constexpr explicit kernel(Source&& src, T value)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend()),
			  _value(std::move(value))
		{}

		raze_always_inline constexpr void operator()() {
			for (; _it != _sent; ++_it, ++_value)
				*_it = std::as_const(_value);
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::iota_result<iterator_type, T> result() {
			return { _source.wrap(std::move(_it)), std::move(_value) };
		}
	};

	template <std::input_or_output_iterator Out, std::sentinel_for<Out> Sent,
		class T = iter_value_or_void_t<Out>>
	constexpr raze_always_inline std::ranges::iota_result<Out, T>
	operator()(Out first, Sent last, T value) const
		requires(std::weakly_incrementable<T> && std::indirectly_writable<Out, const T&>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), std::move(value));
	}

	template <std::ranges::range R, class T = iter_value_or_void_t<std::ranges::iterator_t<R>>>
	constexpr raze_always_inline std::ranges::iota_result<std::ranges::borrowed_iterator_t<R>, T>
	operator()(R&& r, T value) const
		requires(std::weakly_incrementable<T> && std::ranges::output_range<R, const T&>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), std::move(value));
	}
};

/**
 * @ingroup raze_algo_numeric
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Fills a range with sequentially increasing values starting from `value` (C++23).
 *
 * 1) Assigns `value`, `++value`, ... to successive elements of `[first, last)`.
 *
 * 2) Same as (1), but uses `r` as the destination range.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_or_output_iterator Out, std::sentinel_for<Out> Sent,
 *           class T = iter_value_or_void_t<Out> >
 *   requires(std::weakly_incrementable<T> && std::indirectly_writable<Out, const T&>)
 * constexpr std::ranges::iota_result<Out, T>
 * iota( Out first, Sent last, T value );
 *
 * // (2) Range overload
 * template< std::ranges::range R, class T = iter_value_or_void_t<std::ranges::iterator_t<R>> >
 *   requires(std::weakly_incrementable<T> && std::ranges::output_range<R, const T&>)
 * constexpr std::ranges::iota_result<std::ranges::borrowed_iterator_t<R>, T>
 * iota( R&& r, T value );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - destination range to fill
 * - `r`             - destination range to fill
 * - `value`         - starting value (incremented via `++value`; supports braced-init lists via C++26 P2248)
 *
 * ### Return value
 * A `std::ranges::iota_result` `{out, value}` with the past-the-end iterator and final incremented value.
 *
 * ### Complexity
 * Linear: exactly `N` assignments and increments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/numeric/Iota.h>
 *
 * int main() {
 *     std::vector<int> v(5);
 *     raze::algorithm::iota(v, 10);
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fill`
 * - `raze::algorithm::generate`
 */
constexpr inline auto iota = options::function_with_traits<iota_t>;

__RAZE_ALGORITHM_NAMESPACE_END

