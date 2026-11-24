#pragma once

#include <algorithm>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <raze/vx/Algorithm.h>
#include <src/raze/utility/Assert.h>
#include <memory>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto for_each_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()
	.for_icpx<strategy_mode::autovec>()>;

template <class Traits>
struct for_each_t : Traits, dispatchable<for_each_t<Traits>> {
	template <source Source, class F, class Proj>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using vector_value_type = std::iter_value_t<iterator_type>;
		using align = options::alignments<Traits, 1>;
		
		static consteval bool autovectorizable() noexcept {
			return contiguous_source<Source> && !traits::is_iterator_volatile_v<iterator_type>;
		}

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> && modifiable_source<Source> &&
				!traits::is_iterator_volatile_v<iterator_type> &&
				vectorizable_unary_function<F, iterator_type> &&
				vectorizable_projection<Proj, iterator_type>;
		}

		source_type _source;
		unchecked_iterator_type _iterator;
		raze_no_unique_address unchecked_sentinel_type _sentinel;
		raze_no_unique_address F _f;
		raze_no_unique_address Proj _proj;

		constexpr explicit kernel(Source&& src, F f, Proj proj):
			_source(std::forward<Source>(src)), _iterator(_source.ubegin()),
			_sentinel(_source.uend()), _f(std::move(f)), _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()(autovectorizable_tag)
			requires(contiguous_source<Source> && !traits::is_iterator_volatile_v<iterator_type>)
		{
			auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);
			first = align::template assume<0>(first);

			for_noalias(first, last, [&func = _f, &proj = _proj, in = first](auto i)
				raze_always_inline_lambda { std::invoke(func, std::invoke(proj, in[i])); });

			source_type::from_ptr(_iterator, first);
		}

		raze_always_inline constexpr void operator()() {
			raze_novec_loop((; _iterator != _sentinel; ++_iterator), {
				std::invoke(_f, std::invoke(_proj, *_iterator));
			})
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			constexpr auto alignment = align::template policy_for<0, Tag>;

			auto* ptr = std::to_address(_iterator);
			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

			raze_disable_unrolling do {
				auto projected = _proj(vx::load<Tag>[alignment](ptr));
				_f(projected);
				vx::store[alignment](ptr, projected);
				ptr += Tag::size();
			} while (ptr != aligned_end);

			source_type::from_ptr(_iterator, ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			constexpr auto alignment = align::template policy_for<0, Tag>;
			auto* ptr = std::to_address(_iterator);

			auto projected = _proj(vx::load<Tag>[ignore][alignment](ptr));
			_f(projected);
			vx::store[ignore][alignment](ptr, projected);

			advance_bytes(ptr, ignore.tail_bytes());
			source_type::from_ptr(_iterator, ptr);
		}

		constexpr raze_always_inline std::ranges::for_each_result<iterator_type, F> result() const {
			return { _source.wrap(_iterator), _f };
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
			requires(constexpr_sized_source<Source>)
		{
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}

		constexpr void expects() const {
			align::verify(_source);
		}
	};

	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent, class F, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::for_each_result<InIt, F> operator()(
		InIt first, Sent sent, F f, Proj proj = {}) const
			requires(std::indirectly_unary_invocable<F, std::projected<InIt, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(sent)),
			traits::fwd_fn(f), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class F, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::for_each_result<std::ranges::borrowed_iterator_t<R>, F> operator()(
		R&& r, F f, Proj proj = {}) const
			requires(std::indirectly_unary_invocable<F, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(r), traits::fwd_fn(f), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Applies a function object to each element of a range.
 *
 * 1) Applies `f` to the result of dereferencing every iterator in the range `[first, sent)`,
 *    projected by `proj`.
 * 2) Same as (1), but uses `r` as the source range, as if by:
 *    `for_each(std::ranges::begin(r), std::ranges::end(r), std::move(f), proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::input_iterator InIt, std::sentinel_for<InIt> Sent,
 *           class F, class Proj = std::identity >
 *   requires(std::indirectly_unary_invocable<F, std::projected<InIt, Proj>>)
 * constexpr std::ranges::for_each_result<InIt, F>
 * for_each( InIt first, Sent sent, F f, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::input_range R, class F, class Proj = std::identity >
 *   requires(std::indirectly_unary_invocable<F, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::for_each_result<std::ranges::borrowed_iterator_t<R>, F>
 * for_each( R&& r, F f, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to iterate over
 * - `r`             - the range of elements to iterate over
 * - `f`             - unary function object to apply to each projected element
 * - `proj`          - projection to apply to the elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::for_each_result` containing:
 * - `in`: an iterator comparing equal to `sent` (or `std::ranges::end(r)`).
 * - `fun`: the function object `f`.
 *
 * ### Complexity
 * Linear in `N = std::ranges::distance(first, sent)` (or `std::ranges::distance(r)`):
 * `N` applications of `f` and `proj` in the scalar and auto-vectorized paths, or
 * one vector application per SIMD chunk (including the masked tail) in the explicit SIMD path.
 *
 * ### Decorators and Options
 * Configured with `raze::options::unroll<4>` by default (and `strategy_mode::autovec` on GCC, Clang, and ICPX).
 * Supported modifiers via `operator[]`:
 * ```cpp
 * for_each[raze::options::fscalar](...);
 * for_each[raze::options::fautovec](...);
 * for_each[raze::options::fstatic](...);
 * for_each[raze::options::aligned<N>](...);
 * for_each[raze::options::unroll<N>](...);
 * for_each[raze::options::fstatic][raze::options::aligned<N>](...);
 * ```
 *
 * ### Notes
 * - On GCC, Clang, ICPX, AOCC, nvc++ contiguous ranges automatically use the compiler auto-vectorized `for_noalias`
 *   path with runtime CPUID dispatch (`AVX-512`, `AVX2`, `SSE4.2`, baseline) whenever manual `raze::vx` SIMD
 *   is not selected — including for scalar-typed callables (e.g., `[](int& x)`), `const` ranges, and custom
 *   projections.
 * - Explicit `raze::vx` SIMD (used on MSVC, or on GCC/Clang/ICPX when the compile-time target ISA already
 *   satisfies the peak ISA) requires a contiguous modifiable range, a `vx::simd`-compatible callable, and
 *   `std::identity` projection.
 * - `raze::options::fautovec` is a migration aid for generic (`auto`) lambdas with scalar-only bodies
 *   (e.g., `if` branches): it skips `raze::vx::simd` probing and uses compiler auto-vectorization +
 *   dynamic dispatch on GCC, Clang, and ICPX (on MSVC it effectively falls back to scalar execution).
 * - `raze::options::fscalar`, non-contiguous iterators, and constant evaluation (`if consteval`) disable
 *   both SIMD and compiler auto-vectorization, guaranteeing strict sequential in-order scalar execution.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/batch/ForEach.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5};
 *
 *     // Vectorized in-place transformation (`auto&` enables explicit SIMD / autovec)
 *     raze::algorithm::for_each(v, [](auto& x) { x *= 2; });
 *     println(v);
 *
 *     // Scalar-typed lambda on a contiguous range: automatically uses compiler
 *     // auto-vectorization on GCC/Clang/ICPX (scalar fallback on MSVC) without `fautovec`
 *     raze::algorithm::for_each(v, [](int& x) { if (x > 4) x += 1; });
 *     println(v);
 *
 *     // Drop-in migration of a generic (`auto&`) lambda with scalar-only branching:
 *     // `fautovec` skips `vx::simd` probing and routes to autovec (or scalar on MSVC)
 *     raze::algorithm::for_each[raze::options::fautovec](v, [](auto& x) {
 *         if (x > 6) x -= 1;
 *     });
 *     println(v);
 *
 *     // Strict scalar in-order execution (disables both SIMD and compiler autovec)
 *     int sum = 0;
 *     raze::algorithm::for_each[raze::options::fscalar](v, [&sum](const auto& x) { sum += x; });
 *     std::cout << "Sum: " << sum << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 2 4 6 8 10
 * 2 4 7 9 11
 * 2 4 6 8 10
 * Sum: 30
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::for_each_n`
*/
constexpr inline auto for_each = options::function_with_traits<for_each_t>[options::unroll<4>][for_each_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
