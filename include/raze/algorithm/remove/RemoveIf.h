#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <raze/algorithm/find/FindIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto remove_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::manual>()
	.for_clang<strategy_mode::manual>()>;

template <class Traits>
struct remove_if_t : Traits, dispatchable<remove_if_t<Traits>> {
	template <source Source, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
				!traits::is_iterator_volatile_v<iterator_type> &&
				vectorizable_unary_predicate<Predicate, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		source_type _source;
		unchecked_iterator_type _write_iterator;
		unchecked_iterator_type _in_iterator;
		unchecked_sentinel_type _in_sentinel;
		Predicate _predicate;
		Projection _proj;

		constexpr explicit kernel(Source&& source, Predicate pred, Projection proj)
			: _source(std::forward<Source>(source)),
			  _write_iterator(_source.ubegin()), _in_iterator(_source.ubegin()),
			  _in_sentinel(_source.uend()), _predicate(std::move(pred)), _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in_iterator != _in_sentinel; ++_in_iterator) {
				if (!_predicate(_proj(*_in_iterator))) {
					*_write_iterator = std::ranges::iter_move(_in_iterator);
					++_write_iterator;
				}
			}
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_write_iterator);
			const auto aligned_end = bytes_pointer_offset(in_ptr, aligned_size);

			raze_disable_unrolling
			do {
				const auto loaded = vx::load<Tag>(in_ptr);
				const auto mask = _predicate(_proj(loaded));
				out_ptr = vx::compress_store(out_ptr, loaded, mask);
				advance_bytes(in_ptr, sizeof(Tag));
			} while (in_ptr != aligned_end);

			source_type::from_ptr(_in_iterator, in_ptr);
			source_type::from_ptr(_write_iterator, out_ptr);
		}

		constexpr raze_always_inline std::ranges::subrange<iterator_type> result() const {
			return { _source.wrap(_write_iterator), _source.wrap(_in_iterator) };
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() requires(constexpr_sized_source<Source>) {
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}

		static consteval auto targets() {
			if constexpr (sizeof(vector_value_type) >= 4) return algorithm::targets<arch::ISA::AVX512F, arch::ISA::AVX2, arch::ISA::SSSE3>{};
			else return algorithm::targets<arch::ISA::AVX512VBMI2, arch::ISA::AVX2, arch::ISA::SSSE3>{};
		}
	};

	template <std::permutable It, std::sentinel_for<It> Sent, class Pred, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It> operator()(
		It first, Sent last, Pred pred, Proj proj = {}) const
			requires(std::indirect_unary_predicate<Pred, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(find_if[Traits::traits()](first, last, pred, proj),
			std::move(last)), traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class Pred, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R> operator()(
		R&& r, Pred pred, Proj proj = {}) const
			requires(std::permutable<std::ranges::iterator_t<R>> &&
				std::indirect_unary_predicate<Pred, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return (*this)(std::ranges::begin(r), std::ranges::end(r), traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Removes all elements satisfying a predicate from a range in-place.
 *
 * 1) Removes all elements in `[first, last)` for which `bool(std::invoke(pred, std::invoke(proj, *i)))`
 *    evaluates to `true` by shifting the remaining elements forward while preserving their relative order.
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `remove_if(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::permutable It, std::sentinel_for<It> Sent,
 *           class Pred, class Proj = std::identity >
 *   requires(std::indirect_unary_predicate<Pred, std::projected<It, Proj>>)
 * constexpr std::ranges::subrange<It>
 * remove_if( It first, Sent last, Pred pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R,
 *           class Pred, class Proj = std::identity >
 *   requires(std::permutable<std::ranges::iterator_t<R>> &&
 *            std::indirect_unary_predicate<Pred, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * remove_if( R&& r, Pred pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to process
 * - `r`             - the range of elements to process
 * - `pred`          - unary predicate returning `true` for elements to remove
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `[new_end, end)` spanning from the new logical end of the range
 * to the original end (`last` / `std::ranges::end(r)`), ready to be passed to a container's `erase()`.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * applications of `pred` and `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * remove_if[raze::options::fscalar](...);
 * remove_if[raze::options::fstatic](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Uses `raze::algorithm::find_if` to skip the leading elements that are kept before compacting in-place.
 * - Removing is done by shifting elements; the physical size of the underlying container is unchanged.
 * - For contiguous ranges and vectorizable predicates/projections, uses explicit SIMD
 *   `raze::vx::compress_store` (`AVX512F`/`AVX512VBMI2`, `AVX2`, `SSSE3`).
 * - See @ref vectorization_semantics for rules regarding generic (`auto`) vs. scalar-typed predicates.
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/remove/RemoveIf.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5, 6};
 *
 *     auto tail = raze::algorithm::remove_if(v, [](auto x) { return (x & 1) == 0; });
 *     v.erase(tail.begin(), tail.end());
 *
 *     println(v);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 1 3 5
 * ```
*/
constexpr inline auto remove_if = options::function_with_traits<remove_if_t>[remove_strategy];

__RAZE_ALGORITHM_NAMESPACE_END