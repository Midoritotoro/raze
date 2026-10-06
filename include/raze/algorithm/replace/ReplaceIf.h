#pragma once 

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto replace_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct replace_if_t : Traits, dispatchable<replace_if_t<Traits>> {
	template <source Source, class Predicate, class Projection, class Value>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
				vectorizable_unary_predicate<Predicate, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		Source _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		raze_no_unique_address Predicate _predicate;
		raze_no_unique_address Projection _proj;
		Value _new_value;

		constexpr kernel(Source&& src, Predicate pred, Projection proj, Value new_val):
			_source(std::forward<Source>(src)), _predicate(pred), _proj(proj), _new_value(new_val)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}
	
		void operator()(autovectorizable) requires(vectorizable()) {
			auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);

			for (; first != last; ++first)
				*first = _predicate(_proj(*first)) ? _new_value : *first;

			source_type::from_ptr(_iterator, first);
		}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator) 
				*_iterator = (_predicate(_proj(*_iterator))) ? _new_value : *_iterator;
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* ptr = std::to_address(_iterator);

			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);
			const auto new_value = Tag(_new_value);

			raze_disable_unrolling
			do {
				vx::store[_predicate(_proj(vx::load<Tag>(ptr)))](ptr, new_value);
				advance_bytes(ptr, sizeof(Tag));
			} while (ptr != aligned_end);

			source_type::from_ptr(_iterator, ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			auto* ptr = std::to_address(_iterator);
			vx::store[_predicate(_proj(vx::load<Tag>[ignore](ptr))) & ignore()](ptr, Tag(_new_value));
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() requires(constexpr_sized_source<Source>) {
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <std::permutable Iterator, std::sentinel_for<Iterator> Sentinel,
		class Predicate, class Value, class Projection = std::identity>
	constexpr raze_always_inline void operator()(Iterator first, Sentinel sent,
		Predicate pred, Value new_value, Projection proj = {}) const
			requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>>)
	{
		this->dispatch(get_source(std::move(first), std::move(sent)),
			traits::fwd_fn(pred), traits::fwd_fn(proj), new_value);
	}

	template <std::ranges::input_range Range, class Predicate, class Value,
		class Projection = std::identity>
	constexpr raze_always_inline void operator()(Range&& r, Predicate pred,
		Value new_value, Projection proj = {}) const
			requires(std::indirect_unary_predicate<Predicate,
				std::projected<std::ranges::iterator_t<Range>, Projection>>
				&& std::permutable<std::ranges::iterator_t<Range>>)
	{
		this->dispatch(get_source(std::forward<Range>(r)),
			traits::fwd_fn(pred), traits::fwd_fn(proj), new_value);
	}
};

/**
 * @brief Replaces all elements satisfying a specific predicate in a range with a new value.
 *
 * 1) Replaces each element `*i` in `[first, last)` with `new_value` if
 *    `bool(std::invoke(pred, std::invoke(proj, *i)))` evaluates to `true`.
 *
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `replace_if(std::ranges::begin(r), std::ranges::end(r), pred, new_value, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable Iterator, std::sentinel_for<Iterator> Sentinel,
 *           class Predicate, class Value, class Projection = std::identity >
 *   requires std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>>
 * constexpr void replace_if( Iterator first, Sentinel last,
 *                            Predicate pred, Value new_value, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range Range,
 *           class Predicate, class Value, class Projection = std::identity >
 *   requires std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>>
 *         && std::permutable<std::ranges::iterator_t<Range>>
 * constexpr void replace_if( Range&& r,
 *                            Predicate pred, Value new_value, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to modify
 * - `r`           - the range of elements to modify
 * - `pred`        - predicate to apply to the projected elements
 * - `new_value`   - the replacement value
 * - `proj`        - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * (none)
 *
 * ### Complexity
 * Exactly `last - first` (or `std::ranges::distance(r)`) applications of `pred` and `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * replace_if[raze::options::fscalar](...);
 * replace_if[raze::options::unroll<2>](...);
 * replace_if[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - If the source models `contiguous_source`, and both `Predicate` and `Projection` satisfy
 *   vectorization requirements, explicit SIMD vectorization (AVX-512, AVX2, SSE)
 *   or platform-specific autovectorization paths are utilized.
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
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
 *     raze::algorithm::replace_if(v, [](int n) { return n % 2 != 0; }, 0);
 *     println(v);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 5 7 4 2 8 6 1 9 0 3 
 * 0 0 4 2 8 6 0 0 0 0 
 * ```
 */
constexpr inline auto replace_if = options::function_with_traits<replace_if_t>[options::unroll<4>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END