#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/algorithm/find/FindIf.h>
#include <raze/algorithm/copy/Copy.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct remove_copy_if_t : Traits, dispatchable<remove_copy_if_t<Traits>> {
	template <source Source, destination Destination, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using out_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator_type>;
		using out_value_type = destination_value_t<destination_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> && contiguous_destination<Destination> &&
				!traits::is_iterator_volatile_v<iterator_type> &&
				!traits::is_iterator_volatile_v<out_iterator_type> &&
				vectorizable_unary_predicate<Predicate, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>
				&& std::same_as<vector_value_type, out_value_type>;
		}

		source_type _source;
		destination_type _dest;
		unchecked_out_iterator_type _out_it;
		unchecked_iterator_type _in_it;
		unchecked_sentinel_type _in_sent;
		raze_no_unique_address Predicate _predicate;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& source, Destination&& dest, Predicate pred, Projection proj)
			: _source(std::forward<Source>(source)), _dest(std::forward<Destination>(dest)),
			  _out_it(_dest.ubegin()), _in_it(_source.ubegin()),
			  _in_sent(_source.uend()), _predicate(std::move(pred)), _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in_it != _in_sent; ++_in_it) {
				if (!std::invoke(_predicate, std::invoke(_proj, *_in_it))) {
					*_out_it = *_in_it;
					++_out_it;
				}
			}
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* in_ptr = std::to_address(_in_it);
			auto* out_ptr = std::to_address(_out_it);
			const auto aligned_end = bytes_pointer_offset(in_ptr, aligned_size);
			
			raze_disable_unrolling
			do {
				const auto loaded = vx::load<Tag>(in_ptr);
				const auto mask = _predicate(_proj(loaded));
				out_ptr = vx::compress_store(out_ptr, loaded, mask);
				advance_bytes(in_ptr, sizeof(Tag));
			} while (in_ptr != aligned_end);

			source_type::from_ptr(_in_it, in_ptr);
			destination_type::from_ptr(_out_it, out_ptr);
		}

		constexpr raze_always_inline std::ranges::in_out_result<iterator_type, out_iterator_type> result() const {
			return std::ranges::in_out_result<iterator_type, out_iterator_type> { _source.wrap(_in_it), _dest.wrap(_out_it) };
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

	template <std::input_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable OutIt, 
		class Pred, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::in_out_result<It, OutIt> operator()(
		It first, Sent last, OutIt out, Pred pred, Proj proj = {}) const
			requires(std::indirectly_copyable<It, OutIt> && std::indirect_unary_predicate<
				Pred, std::projected<It, Proj>>)
	{
		if constexpr (std::forward_iterator<It>) {
			auto found = find_if[options::drop_alignment(Traits::traits())](first, last, pred, proj);
			auto prefix = copy[Traits::traits()](std::move(first), found, std::move(out));
			if (found == last) return { std::move(found), std::move(prefix.out) };
			return this->dispatch(get_source(std::ranges::next(std::move(found)), std::move(last)),
				get_destination(std::move(prefix.out)), traits::fwd_fn(pred), traits::fwd_fn(proj));
		}
		else {
			return this->dispatch(get_source(std::move(first), std::move(last)),
				get_destination(std::move(out)), traits::fwd_fn(pred), traits::fwd_fn(proj));
		}
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt, class Pred, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, OutIt> operator()(
		R&& r, OutIt out, Pred pred, Proj proj = {}) const
			requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> && std::indirect_unary_predicate<
				Pred, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(out),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};


/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Copies elements from a range to another range, omitting elements satisfying a predicate.
 *
 * 1) Copies all elements from `[first, last)` to the destination range beginning at `out`,
 *    omitting those for which `bool(std::invoke(pred, std::invoke(proj, *i)))` evaluates to `true`.
 *    Relative order of the copied elements is preserved.
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `remove_copy_if(std::ranges::begin(r), std::ranges::end(r), std::move(out), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator It, std::sentinel_for<It> Sent,
 *           std::weakly_incrementable OutIt, class Pred, class Proj = std::identity >
 *   requires(std::indirectly_copyable<It, OutIt> &&
 *            std::indirect_unary_predicate<Pred, std::projected<It, Proj>>)
 * constexpr std::ranges::in_out_result<It, OutIt>
 * remove_copy_if( It first, Sent last, OutIt out, Pred pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, std::weakly_incrementable OutIt,
 *           class Pred, class Proj = std::identity >
 * constexpr std::ranges::in_out_result<std::ranges::iterator_t<R>, OutIt>
 * remove_copy_if( R&& r, OutIt out, Pred pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the range of elements to copy from
 * - `r`             - the range of elements to copy from
 * - `out`           - the beginning of the destination range
 * - `pred`          - unary predicate returning `true` for elements to omit
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::in_out_result` containing:
 * - `in`: an iterator comparing equal to `last` (or `std::ranges::end(r)`).
 * - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * Linear: exactly `std::ranges::distance(first, last)` (or `std::ranges::distance(r)`)
 * applications of `pred` and `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * remove_copy_if[raze::options::fscalar](...);
 * remove_copy_if[raze::options::fstatic](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Optimizes the leading non-matching prefix by locating the first match via `raze::algorithm::find_if`
 *   and bulk-copying the prefix with `raze::algorithm::copy` before entering the compaction loop.
 * - For contiguous buffers of matching element types and vectorizable predicates/projections,
 *   uses explicit SIMD `raze::vx::compress_store` (`AVX512F`/`AVX512VBMI2`, `AVX2`, `SSSE3`).
 * - See @ref vectorization_semantics for rules regarding generic (`auto`) vs. scalar-typed predicates.
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/remove/RemoveCopyIf.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     const std::vector<int> src{1, 2, 3, 4, 5, 6};
 *     std::vector<int> odds(src.size());
 *
 *     auto [in, out] = raze::algorithm::remove_copy_if(
 *         src, odds.begin(), [](auto x) { return (x & 1) == 0; });
 *     odds.erase(out, odds.end());
 *
 *     println(odds);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 1 3 5
 * ```
*/
constexpr inline auto remove_copy_if = options::function_with_traits<remove_copy_if_t>;

__RAZE_ALGORITHM_NAMESPACE_END