#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct uninitialized_fill_t : Traits, dispatchable<uninitialized_fill_t<Traits>> {
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
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _cur;
		const T& _value;

		constexpr explicit kernel(Source&& src, const T& value)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _cur(_first),
			  _value(value)
		{}

		raze_always_inline constexpr void operator()() {
			detail::uninit_destroy_guard<unchecked_iterator_type> guard{ _first, _cur };
			for (; _cur != _sent; ++_cur)
				std::construct_at(std::addressof(*_cur), _value);
			guard.active = false;
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_cur));
		}
	};

	template <detail::nothrow_forward_iterator It, detail::nothrow_sentinel_for<It> Sent,
		class T = std::iter_value_t<It>>
	constexpr raze_always_inline It operator()(It first, Sent last, const T& x) const
		requires(std::constructible_from<std::iter_value_t<It>, const T&>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), x);
	}

	template <detail::nothrow_forward_range R, class T = std::ranges::range_value_t<R>>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
	operator()(R&& r, const T& x) const
		requires(std::constructible_from<std::ranges::range_value_t<R>, const T&>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), x);
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Constructs copies of a value throughout an uninitialized memory range.
 *
 * 1) Constructs every element in `[first, last)` from `x` via `std::construct_at`. Destroys all
 *    previously constructed elements if an exception is thrown.
 *
 * 2) Same as (1), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< nothrow_forward_iterator It, nothrow_sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It> >
 *   requires(std::constructible_from<std::iter_value_t<It>, const T&>)
 * constexpr It uninitialized_fill( It first, Sent last, const T& x );
 *
 * // (2) Range overload
 * template< nothrow_forward_range R, class T = std::ranges::range_value_t<R> >
 *   requires(std::constructible_from<std::ranges::range_value_t<R>, const T&>)
 * constexpr std::ranges::borrowed_iterator_t<R> uninitialized_fill( R&& r, const T& x );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - range of uninitialized storage
 * - `x`                  - value to copy-construct (supports braced-init lists via C++26 P2248)
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear in `N`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::uninitialized_fill_n`
 * - `raze::algorithm::fill`
 * - `raze::algorithm::destroy`
 */
constexpr inline auto uninitialized_fill = options::function_with_traits<uninitialized_fill_t>;

__RAZE_ALGORITHM_NAMESPACE_END

