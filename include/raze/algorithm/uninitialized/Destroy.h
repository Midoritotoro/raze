#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct destroy_t : Traits, dispatchable<destroy_t<Traits>> {
	template <source Source>
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

		constexpr explicit kernel(Source&& src)
			: _source(std::forward<Source>(src)),
			  _it(_source.ubegin()),
			  _sent(_source.uend())
		{}

		raze_always_inline constexpr void operator()() noexcept {
			if constexpr (std::is_trivially_destructible_v<std::iter_value_t<unchecked_iterator_type>>) {
				std::ranges::advance(_it, _sent);
			}
			else {
				for (; _it != _sent; ++_it)
					std::destroy_at(std::addressof(*_it));
			}
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_it));
		}
	};

	template <detail::nothrow_input_iterator It, detail::nothrow_sentinel_for<It> Sent>
	constexpr raze_always_inline It operator()(It first, Sent last) const noexcept
		requires(std::destructible<std::iter_value_t<It>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)));
	}

	template <detail::nothrow_input_range R>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r) const noexcept
		requires(std::destructible<std::ranges::range_value_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)));
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Destroys all objects in a range.
 *
 * 1) Destroys every object in `[first, last)` via `std::destroy_at(std::addressof(*i))`.
 *
 * 2) Same as (1), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< nothrow_input_iterator It, nothrow_sentinel_for<It> Sent >
 *   requires(std::destructible<std::iter_value_t<It>>)
 * constexpr It destroy( It first, Sent last ) noexcept;
 *
 * // (2) Range overload
 * template< nothrow_input_range R >
 *   requires(std::destructible<std::ranges::range_value_t<R>>)
 * constexpr std::ranges::borrowed_iterator_t<R> destroy( R&& r ) noexcept;
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - range of objects to destroy
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear in `N` (constant for trivially destructible types with sized sentinels).
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options.
 *
 * ### See also
 * - `raze::algorithm::destroy_n`
 * - `raze::algorithm::destroy_at`
 */
constexpr inline auto destroy = options::function_with_traits<destroy_t>;

__RAZE_ALGORITHM_NAMESPACE_END

