#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/memory/Memset.h>
#include <raze/vx/Algorithm.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto fill_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::manual>()
	.for_clang<strategy_mode::autovec>()
	.for_icpx<strategy_mode::autovec>()>;

template <class Traits>
struct fill_t : Traits, dispatchable<fill_t<Traits>> {
	template <source Source, class T>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using value_type = iter_value_or_void_t<iterator_type>;
		using align = options::alignments<Traits, 1>;

		static consteval auto deduce_value_type() noexcept {
			if constexpr (std::is_void_v<value_type>) return void();
			else if constexpr (std::floating_point<value_type> && sizeof(value_type) <= 8) return value_type{};
			else if constexpr (sizeof(value_type) <= 8 && sizeof(value_type) != 0 && ((sizeof(value_type) & (sizeof(value_type) - 1)) == 0))
				return typename IntegerForSizeof<value_type>::Signed{};
			else return void();
		}

		using vector_value_type = decltype(deduce_value_type());

		static consteval bool native() noexcept {
			return vectorizable() && vx::arithmetic_type<T> &&
				std::same_as<T, value_type> && sizeof(T) == 1;
		}

		static consteval bool vectorizable() noexcept {
			if constexpr (std::is_void_v<value_type> || std::is_void_v<vector_value_type>) {
				return false;
			}
			else {
				return contiguous_source<Source> &&
					!traits::is_iterator_volatile_v<iterator_type> &&
					std::is_trivially_copyable_v<value_type> &&
					std::is_trivially_assignable_v<std::iter_reference_t<iterator_type>, const T&> &&
					std::convertible_to<const T&, value_type> &&
					(sizeof(value_type) <= 8) && (sizeof(value_type) != 0) &&
					((sizeof(value_type) & (sizeof(value_type) - 1)) == 0);
			}
		}

		raze_no_unique_address source_type _source;
		raze_no_unique_address unchecked_iterator_type _iterator;
		raze_no_unique_address unchecked_sentinel_type _sentinel;
		const T& _value;

		constexpr explicit kernel(Source&& source, const T& value)
			: _source(std::forward<Source>(source)), _iterator(_source.ubegin()), 
			_sentinel(_source.uend()), _value(value)
		{}

		constexpr void expects() const {
			align::verify(_source);
		}

		raze_always_inline constexpr void operator()(autovectorizable_tag) requires(vectorizable()) {
			const value_type v = static_cast<value_type>(_value);
			auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);
			first = align::template assume<0>(first);

			for_noalias(first, last, [v, out = first](auto i) raze_always_inline_lambda {
				out[i] = v;
			});

			source_type::from_ptr(_iterator, first);
		}

		raze_always_inline constexpr void operator()() {
			raze_novec_loop((; _iterator != _sentinel; ++_iterator), {
				*_iterator = _value;
			})
		}

		raze_always_inline void operator()(native_tag) requires(native()) {
			auto* ptr = std::to_address(_iterator);
			__raze_memset(ptr, static_cast<u8>(_value), size());
			source_type::from_ptr(_iterator, bytes_pointer_offset(ptr, size()));
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			constexpr auto policy = align::template policy_for<0, Tag>;
			auto* ptr = std::to_address(_iterator);
			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

			const Tag broadcasted = math::bit_cast<vector_value_type>(static_cast<value_type>(_value));

			raze_disable_unrolling do {
				vx::store[policy](ptr, broadcasted);
				advance_bytes(ptr, sizeof(Tag));
			} while (ptr != aligned_end);

			source_type::from_ptr(_iterator, ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			if (ignore.tail_bytes() == 0) return;

			constexpr auto policy = align::template policy_for<0, Tag>;
			auto* ptr = std::to_address(_iterator);
			vx::store[ignore][policy](ptr, Tag(math::bit_cast<vector_value_type>(static_cast<value_type>(_value))));

			advance_bytes(ptr, ignore.tail_bytes());
			source_type::from_ptr(_iterator, ptr);
		}

		constexpr raze_always_inline iterator_type result() const {
			return _source.wrap(_iterator);
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() requires(constexpr_sized_source<Source>) {
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <std::input_or_output_iterator It, std::sentinel_for<It> Sent, class T = iter_value_or_void_t<It>>
	constexpr raze_always_inline It operator()(It first, Sent last, const T& value) const
		requires(std::output_iterator<It, const T&>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), value);
	}

	template <std::ranges::range R, class T = iter_value_or_void_t<std::ranges::iterator_t<R>>>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(
		R&& r, const T& value) const
			requires(std::ranges::output_range<R, const T&>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), value);
	}
};

/**
 * @ingroup raze_algo_fill
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Assigns the given value to elements in a range.
 *
 * 1) Assigns `value` to every element in the range `[first, last)`.
 *
 * 2) Same as (1), for pure output iterators that are not `std::indirectly_readable`.
 *
 * 3) Same as (1), but uses `r` as the destination range, as if by:
 *    `fill(std::ranges::begin(r), std::ranges::end(r), value)`.
 *
 * 4) Same as (3), for pure output ranges whose iterator is not `std::indirectly_readable`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Readable iterator-sentinel overload (defaulted value type)
 * template< std::input_or_output_iterator It, std::sentinel_for<It> Sent,
 *           class T = std::iter_value_t<It> >
 *   requires(std::indirectly_readable<It> && std::output_iterator<It, const T&>)
 * constexpr It fill( It first, Sent last, const std::type_identity_t<T>& value );
 *
 * // (2) Pure output iterator-sentinel overload
 * template< std::input_or_output_iterator It, std::sentinel_for<It> Sent, class T >
 *   requires(!std::indirectly_readable<It> && std::output_iterator<It, const T&>)
 * constexpr It fill( It first, Sent last, const T& value );
 *
 * // (3) Readable range overload (defaulted value type)
 * template< class R, class T = std::ranges::range_value_t<R> >
 *   requires(std::indirectly_readable<std::ranges::iterator_t<R>> &&
 *            std::ranges::output_range<R, const T&>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * fill( R&& r, const std::type_identity_t<T>& value );
 *
 * // (4) Pure output range overload
 * template< std::ranges::range R, class T >
 *   requires(!std::indirectly_readable<std::ranges::iterator_t<R>> &&
 *            std::ranges::output_range<R, const T&>)
 * constexpr std::ranges::borrowed_iterator_t<R>
 * fill( R&& r, const T& value );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to modify
 * - `r`             - the destination range
 * - `value`         - the value to be assigned
 *
 * ### Return value
 * An iterator comparing equal to `last` (or `std::ranges::end(r)`).
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`) assignments.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`; `fill_strategy` (`strategy_mode::manual` on GCC,
 *   `strategy_mode::autovec` on Clang and ICPX).
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous destination range (1 range).
 * - **Auto-vectorization:** Provides an `autovec` kernel for vectorizable contiguous buffers (see @ref autovec_behavior).
 *
 * ### Notes
 * - Contiguous ranges of 1-byte arithmetic types matching `value` take the native `memset` fast path.
 * - Explicit `raze::vx` SIMD broadcasts `value` for contiguous trivially copyable types of power-of-two size
 *   up to 8 bytes, with masked vector stores for remaining tail elements.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/fill/Fill.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v(8);
 *
 *     // Fill entire vector with 42
 *     raze::algorithm::fill(v, 42);
 *     println(v);
 *
 *     // Fill subrange [begin, begin + 4) with -1
 *     raze::algorithm::fill(v.begin(), v.begin() + 4, -1);
 *     println(v);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 42 42 42 42 42 42 42 42
 * -1 -1 -1 -1 42 42 42 42
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::fill_n`
*/
constexpr inline auto fill = options::function_with_traits<fill_t>[options::unroll<4>][fill_strategy];

__RAZE_ALGORITHM_NAMESPACE_END