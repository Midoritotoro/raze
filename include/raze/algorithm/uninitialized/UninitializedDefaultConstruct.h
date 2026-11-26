#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>
#include <new>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct uninitialized_default_construct_t : Traits, dispatchable<uninitialized_default_construct_t<Traits>> {
	template <source Source>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using value_type = std::remove_reference_t<std::iter_reference_t<unchecked_iterator_type>>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _cur;

		constexpr explicit kernel(Source&& src)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _cur(_first)
		{}

		raze_always_inline constexpr void operator()() {
			if constexpr (std::is_trivially_default_constructible_v<value_type>) {
				if !consteval {
					std::ranges::advance(_cur, _sent);
					return;
				}
			}

			detail::uninit_destroy_guard<unchecked_iterator_type> guard{ _first, _cur };
			for (; _cur != _sent; ++_cur) {
				if consteval {
					std::construct_at(std::addressof(*_cur));
				}
				else {
					::new (static_cast<void*>(std::addressof(*_cur))) value_type;
				}
			}
			guard.active = false;
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() {
			return _source.wrap(std::move(_cur));
		}
	};

	template <detail::nothrow_forward_iterator It, detail::nothrow_sentinel_for<It> Sent>
	constexpr raze_always_inline It operator()(It first, Sent last) const
		requires(std::default_initializable<std::iter_value_t<It>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)));
	}

	template <detail::nothrow_forward_range R>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r) const
		requires(std::default_initializable<std::ranges::range_value_t<R>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)));
	}
};

/**
 * @ingroup raze_algo_uninitialized
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Default-constructs objects in an uninitialized memory range with strong exception safety.
 *
 * 1) Default-initializes every element in `[first, last)`. If an exception is thrown, all previously
 *    constructed objects are destroyed before propagating the exception.
 *
 * 2) Same as (1), using range `r`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< nothrow_forward_iterator It, nothrow_sentinel_for<It> Sent >
 *   requires(std::default_initializable<std::iter_value_t<It>>)
 * constexpr It uninitialized_default_construct( It first, Sent last );
 *
 * // (2) Range overload
 * template< nothrow_forward_range R >
 *   requires(std::default_initializable<std::ranges::range_value_t<R>>)
 * constexpr std::ranges::borrowed_iterator_t<R> uninitialized_default_construct( R&& r );
 * ```
 *
 * ### Parameters
 * - `first`, `last`, `r` - range of uninitialized storage
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
 * - `raze::algorithm::uninitialized_default_construct_n`
 * - `raze::algorithm::uninitialized_value_construct`
 * - `raze::algorithm::destroy`
 */
constexpr inline auto uninitialized_default_construct =
	options::function_with_traits<uninitialized_default_construct_t>;

__RAZE_ALGORITHM_NAMESPACE_END

