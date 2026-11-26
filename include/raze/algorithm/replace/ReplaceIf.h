#pragma once 

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <raze/vx/Algorithm.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto replace_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()
	.for_icpx<strategy_mode::autovec>()>;

template <class Traits>
struct replace_if_t : Traits, dispatchable<replace_if_t<Traits>> {
	template <source Source, class Predicate, class Projection, class Value>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<iterator_type>;
		using align = options::alignments<Traits, 1>;
		static_assert(sizeof(align) > 0);

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> && modifiable_source<Source> &&
				!traits::is_iterator_volatile_v<iterator_type> &&
				std::convertible_to<const Value&, vector_value_type> &&
				vectorizable_unary_predicate<Predicate, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _iterator;
		raze_no_unique_address unchecked_sentinel_type _sentinel;
		raze_no_unique_address Predicate _predicate;
		raze_no_unique_address Projection _proj;
		const Value& _new_value;

		constexpr kernel(Source&& src, Predicate pred, Projection proj, const Value& new_val):
			_source(std::forward<Source>(src)), _iterator(_source.ubegin()), _sentinel(_source.uend()),
			_predicate(std::move(pred)), _proj(std::move(proj)), _new_value(new_val)
		{}

		constexpr void expects() const {
			align::verify(_source);
		}

		raze_always_inline constexpr void operator()(autovectorizable_tag)
			requires(contiguous_source<Source> && modifiable_source<Source> &&
				!traits::is_iterator_volatile_v<iterator_type>)
		{
			auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);
			first = align::template assume<0>(first);

			std::conditional_t<traits::lightweight_callable<Predicate>, Predicate, Predicate&> pred = _predicate;
			std::conditional_t<traits::lightweight_callable<Projection>, Projection, Projection&> proj = _proj;
			std::conditional_t<std::is_trivially_copyable_v<Value> && sizeof(Value) <= sizeof(void*) * 2, Value, const Value&> newv = _new_value;

			for_noalias(first, last, [&pred, &proj, &newv,
				in = first](auto i) raze_always_inline_lambda {
					if (std::invoke(pred, std::invoke(proj, in[i])))
						in[i] = newv;
				});

			source_type::from_ptr(_iterator, first);
		}

		raze_always_inline constexpr void operator()() {
			raze_novec_loop((; _iterator != _sentinel; ++_iterator), {
				if (std::invoke(_predicate, std::invoke(_proj, *_iterator)))
					*_iterator = _new_value;
			})
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			constexpr auto alignment = align::template policy_for<0, Tag>;
			auto* ptr = std::to_address(_iterator);

			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);
			const auto broadcasted = Tag(static_cast<vector_value_type>(_new_value));

			raze_disable_unrolling do {
				const auto data = vx::load<Tag>[alignment](ptr);
				vx::store[alignment](ptr, vx::select[_predicate(_proj(data)), data](broadcasted));
				advance_bytes(ptr, sizeof(Tag));
			} while (ptr != aligned_end);

			source_type::from_ptr(_iterator, ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			if (ignore.tail_bytes() == 0) return;

			constexpr auto alignment = align::template policy_for<0, Tag>;
			auto* ptr = std::to_address(_iterator);
			const auto mask = _predicate(_proj(vx::load<Tag>[ignore][alignment](ptr))) & ignore();
			vx::store[mask][alignment](ptr, Tag(static_cast<vector_value_type>(_new_value)));

			advance_bytes(ptr, ignore.tail_bytes());
			source_type::from_ptr(_iterator, ptr);
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() requires(constexpr_sized_source<Source>) {
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel,
		class Predicate, class Value, class Projection = std::identity>
	constexpr raze_always_inline void operator()(Iterator first, Sentinel sent,
		Predicate pred, const Value& new_value, Projection proj = {}) const
			requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>> &&
				std::indirectly_writable<Iterator, const Value&>)
	{
		this->dispatch(get_source(std::move(first), std::move(sent)),
			traits::fwd_fn(pred), traits::fwd_fn(proj), new_value);
	}

	template <std::ranges::input_range Range, class Predicate, class Value,
		class Projection = std::identity>
	constexpr raze_always_inline void operator()(Range&& r, Predicate pred,
		const Value& new_value, Projection proj = {}) const
			requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> &&
				std::indirectly_writable<std::ranges::iterator_t<Range>, const Value&>)
	{
		this->dispatch(get_source(r),
			traits::fwd_fn(pred), traits::fwd_fn(proj), new_value);
	}
};

/**
 * @ingroup raze_algo_replace
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Replaces all elements satisfying a specific predicate in a range with a new value.
 *
 * 1) Replaces each element `*i` in `[first, sent)` with `new_value` if
 *    `bool(std::invoke(pred, std::invoke(proj, *i)))` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `replace_if(std::ranges::begin(r), std::ranges::end(r), pred, new_value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel,
 *           class Predicate, class Value, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>> &&
 *            std::indirectly_writable<Iterator, const Value&>)
 * constexpr void replace_if( Iterator first, Sentinel sent,
 *                            Predicate pred, const Value& new_value, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range Range,
 *           class Predicate, class Value, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> &&
 *            std::indirectly_writable<std::ranges::iterator_t<Range>, const Value&>)
 * constexpr void replace_if( Range&& r,
 *                            Predicate pred, const Value& new_value, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to modify
 * - `r`             - the range of elements to modify
 * - `pred`          - unary predicate returning `true` for elements to replace
 * - `new_value`     - the replacement value
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * (none)
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, sent)` (or `std::ranges::distance(r)`)
 * applications of `pred` and `proj`.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **Defaults:** `raze::options::unroll<4>`; `replace_strategy` (`strategy_mode::autovec` on GCC, Clang, and ICPX).
 * - **`raze::options::aligned<N>`:** Declares alignment for the contiguous range (1 range).
 * - **Auto-vectorization:** Provides an `autovec` kernel (see @ref autovec_behavior).
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD uses `raze::vx::select` for full chunks and masked vector stores for remaining tail elements.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/replace/ReplaceIf.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
 *     println(v);
 *
 *     // Replace all odd numbers with 0
 *     raze::algorithm::replace_if(v, [](auto n) { return (n & 1) != 0; }, 0);
 *     println(v);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 5 7 4 2 8 6 1 9 0 3
 * 0 0 4 2 8 6 0 0 0 0
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::replace`
 * - `raze::algorithm::replace_copy_if`
 * - `raze::algorithm::replace_copy`
*/
constexpr inline auto replace_if = options::function_with_traits<replace_if_t>[options::unroll<4>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END