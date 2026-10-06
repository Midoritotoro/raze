#pragma once

#include <algorithm>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto for_each_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct for_each_t : Traits, dispatchable<for_each_t<Traits>> {
	template <modifiable_source Source, class F, class Proj>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using vector_value_type = std::iter_value_t<iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
				vectorizable_unary_function<F, iterator_type> &&
				vectorizable_projection<Proj, iterator_type>;
		}

		source_type _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		raze_no_unique_address F _f;
		raze_no_unique_address Proj _proj;

		constexpr explicit kernel(Source&& src, F f, Proj proj):
			_source(std::forward<Source>(src)), _iterator(_source.ubegin()),
			_sentinel(_source.uend()), _f(f), _proj(proj)
		{}

		raze_always_inline constexpr void operator()(autovectorizable) requires(vectorizable()) {
			for (auto [first, last] = source_type::to_raw_range(_iterator, _sentinel); first != last; ++first)
				_f(_proj(*first));
		}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator)
				_f(_proj(*_iterator));
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* ptr = std::to_address(_iterator);
			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

			raze_disable_unrolling
			do {
				auto projected = _proj(vx::load<Tag>(ptr));
				_f(projected);
				vx::store(ptr, projected);
				advance_bytes(ptr, sizeof(Tag));
			} while (ptr != aligned_end);

			source_type::from_ptr(_iterator, ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			auto* ptr = std::to_address(_iterator);

			auto projected = _proj(vx::load<Tag>[ignore](ptr));
			_f(projected);
			vx::store[ignore](ptr, projected);

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
	constexpr raze_always_inline std::ranges::for_each_result<std::ranges::iterator_t<R>, F> operator()(R&& r, F f, Proj proj = {}) const
		requires(std::indirectly_unary_invocable<F, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(f), traits::fwd_fn(proj));
	}
};

/**
 * @brief Applies a function object to the result of dereferencing each iterator in a range, in order.
 *
 * 1) Applies `f` to the result of dereferencing every iterator in the range `[first, sent)`,
 *    projected by `proj`.
 *
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
 * constexpr std::ranges::for_each_result<std::ranges::iterator_t<R>, F>
 * for_each( R&& r, F f, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `sent` - the range of elements to iterate over
 * - `r`           - the range of elements to iterate over
 * - `f`           - unary function object to apply to projected elements
 * - `proj`        - projection to apply to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::for_each_result` containing:
 * - `in`: an iterator pointing to the end of the input range (`sent`).
 * - `fun`: the function object `f` passed as an argument.
 *
 * ### Complexity
 * Linear: exactly `sent - first` (or `std::ranges::distance(r)`) invocations of `f` and applications of `proj`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * for_each[raze::options::fscalar](...);
 * for_each[raze::options::unroll<2>](...);
 * for_each[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/for_each/ForEach.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5};
 *
 *     // In-place modification: double each element
 *     raze::algorithm::for_each(v, [](int& x) { x *= 2; });
 *     println(v);
 *
 *     // Non-modifying traversal with side effects
 *     int sum = 0;
 *     raze::algorithm::for_each(v, [&sum](int x) { sum += x; });
 *     std::cout << "Sum: " << sum << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 2 4 6 8 10 
 * Sum: 30
 * ```
 */
constexpr inline auto for_each = options::function_with_traits<for_each_t>[options::unroll<4>][for_each_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
