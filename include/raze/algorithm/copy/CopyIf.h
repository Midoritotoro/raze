#pragma once 

#include <raze/algorithm/remove/RemoveCopyIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct copy_if_t : Traits {
	template <std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel,
		std::weakly_incrementable Output, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline auto operator()(Iterator first, Sentinel last,
		Output output, Predicate pred, Projection proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>> &&
			std::indirectly_copyable<Iterator, Output>)
	{
		return raze::algorithm::remove_copy_if[Traits::traits()](std::move(first), std::move(last),
			std::move(output), algorithm::make_not_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range Range, std::weakly_incrementable Output,
		class Predicate, class Projection = std::identity>
	constexpr raze_always_inline auto operator()(Range&& range, Output output, Predicate pred, Projection proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> 
			&& std::indirectly_copyable<std::ranges::iterator_t<Range>, Output>)
	{
		return raze::algorithm::remove_copy_if[Traits::traits()](std::forward<Range>(range),
			std::move(output), algorithm::make_not_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies elements from a range that satisfy a predicate.
 *
 * 1) Copies all elements in `[first, last)` for which `bool(std::invoke(pred, std::invoke(proj, *i)))`
 *    evaluates to `true` into the destination range beginning at `output`, preserving their relative order.
 * 2) Same as (1), but uses `range` as the source range, as if by:
 *    `copy_if(std::ranges::begin(range), std::ranges::end(range), std::move(output), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel,
 *           std::weakly_incrementable Output, class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>> &&
 *            std::indirectly_copyable<Iterator, Output>)
 * constexpr std::ranges::in_out_result<Iterator, Output>
 * copy_if( Iterator first, Sentinel last, Output output,
 *          Predicate pred, Projection proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range Range, std::weakly_incrementable Output,
 *           class Predicate, class Projection = std::identity >
 *   requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> &&
 *            std::indirectly_copyable<std::ranges::iterator_t<Range>, Output>)
 * constexpr std::ranges::in_out_result<std::ranges::iterator_t<Range>, Output>
 * copy_if( Range&& range, Output output,
 *          Predicate pred, Projection proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the source range of elements to copy from
 * - `range`         - the source range of elements to copy from
 * - `output`        - the beginning of the destination range (must be able to accommodate all matching elements)
 * - `pred`          - unary predicate returning `true` for the elements to keep
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::in_out_result` containing:
 * - `in`: an iterator comparing equal to `last` (or `std::ranges::end(range)`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(range)`)
 * applications of `pred` and `proj`, and at most as many element copies.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]` (forwarded to `raze::algorithm::remove_copy_if`):
 * ```cpp
 * copy_if[raze::options::fscalar](...);
 * copy_if[raze::options::fstatic](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - The behavior is undefined if the source and destination ranges overlap.
 * - Implemented by delegating to `raze::algorithm::remove_copy_if` with a negated predicate
 *   (preserving SIMD vectorizability for vectorizable predicates).
 * - Execution path selection:
 *   - **Constant evaluation (`if consteval`) or `fscalar`**: always executes via the scalar fallback path.
 *   - **SIMD path**: for contiguous source and destination ranges of matching element types with
 *     vectorizable predicates and projections, uses explicit SIMD `raze::vx::compress_store`
 *     (`AVX-512`, `AVX2`, `SSSE3`). Prefer pre-allocating a contiguous destination buffer (as shown
 *     below) over output adapters like `std::back_inserter` to enable vectorization.
 *   - **Scalar fallback**: used for non-contiguous iterators (e.g., `std::back_inserter`),
 *     non-vectorizable predicates/projections, or mismatched element types.
 * - See @ref vectorization_semantics for rules regarding generic (`auto`) vs. scalar-typed predicates.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/copy/CopyIf.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     const std::vector<int> src{1, 2, 3, 4, 5, 6};
 *
 *     // Pre-allocate contiguous storage to enable the SIMD compress_store path
 *     std::vector<int> evens(src.size());
 *
 *     auto [in, out] = raze::algorithm::copy_if(
 *         src, evens.begin(), [](auto x) { return (x & 1) == 0; });
 *     evens.erase(out, evens.end());
 *
 *     println(evens);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 2 4 6
 * ```
*/
constexpr inline auto copy_if = raze::options::function_with_traits<copy_if_t>;

__RAZE_ALGORITHM_NAMESPACE_END