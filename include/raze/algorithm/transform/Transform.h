#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/Destination.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto transform_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct transform_t : Traits, dispatchable<transform_t<Traits>> {
	template <class... Ts>
	struct kernel;

	template <source Source, destination Destination, class Function, class Projection>
	struct kernel<Source, Destination, Function, Projection> {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using destination_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_destination_type = typename destination_type::unchecked_iterator_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> && contiguous_destination<Destination> &&
				vectorizable_unary_function<Function, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		destination_type _destination;
		source_type _source;
		unchecked_iterator_type _in_iterator;
		unchecked_sentinel_type _in_sentinel;
		unchecked_destination_type _out_iterator;
		Function _function;
		Projection _proj;

		constexpr explicit kernel(Source&& source, Destination&& dest, Function f, Projection proj)
			: _source(std::forward<Source>(source)),
			  _destination(std::forward<Destination>(dest)),
			  _function(f), _proj(proj),
			  _out_iterator(_destination.ubegin()),
			  _in_iterator(_source.ubegin()), _in_sentinel(_source.uend())
		{}

		raze_always_inline constexpr void operator()(autovectorizable) requires(vectorizable()) {
			auto* raze_restrict in_ptr = std::to_address(_in_iterator);
			auto* raze_restrict in_last = std::to_address(_in_sentinel);
			auto* raze_restrict out_ptr = std::to_address(_out_iterator);

			for (; in_ptr != in_last; ++in_ptr, ++out_ptr)
				*out_ptr = _function(_proj(*in_ptr));

			source_type::from_ptr(_in_iterator, in_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in_iterator != _in_sentinel; ++_in_iterator, ++_out_iterator)
				*_out_iterator = _function(_proj(*_in_iterator));
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);
			const auto aligned_end = bytes_pointer_offset(in_ptr, aligned_size);

			do {
				vx::store(out_ptr, _function(_proj(vx::load<Tag>(in_ptr))));
				advance_bytes(in_ptr, out_ptr, sizeof(Tag));
			} while (in_ptr != aligned_end);

			source_type::from_ptr(_in_iterator, in_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);

			const auto data = vx::load<Tag>(in_ptr);
			vx::store[ignore](out_ptr, _function(_proj(data)));

			advance_bytes(in_ptr, out_ptr, ignore.tail_bytes());

			source_type::from_ptr(_in_iterator, in_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		constexpr raze_always_inline std::ranges::unary_transform_result<iterator_type, destination_iterator_type> result() const {
			return { _source.wrap(_in_iterator), _destination.wrap(_out_iterator) };
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() requires(constexpr_sized_source<Source>) {
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <source Source1, source Source2, destination Destination, class Function, class Projection1, class Projection2>
	struct kernel<Source1, Source2, Destination, Function, Projection1, Projection2> {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator1_type = typename source1_type::iterator_type;
		using iterator2_type = typename source2_type::iterator_type;
		using destination_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;
		using unchecked_destination_type = typename destination_type::unchecked_iterator_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator1_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source1> && contiguous_source<Source2> && contiguous_destination<Destination> &&
				vectorizable_binary_function<Function, iterator1_type, iterator2_type> &&
				vectorizable_projection<Projection1, iterator1_type> &&
				vectorizable_projection<Projection2, iterator2_type>;
		}

		destination_type _destination;
		source1_type _source1;
		source2_type _source2;
		unchecked_iterator1_type _in1_iterator;
		unchecked_sentinel1_type _in1_sentinel;
		unchecked_iterator2_type _in2_iterator;
		unchecked_sentinel2_type _in2_sentinel;
		unchecked_destination_type _out_iterator;
		Function _function;
		Projection1 _proj1;
		Projection2 _proj2;

		constexpr explicit kernel(Source1&& source1, Source2&& source2, Destination&& dest,
		         Function f, Projection1 proj1, Projection2 proj2)
			: _source1(std::forward<Source1>(source1)),
			  _source2(std::forward<Source2>(source2)),
			  _destination(std::forward<Destination>(dest)),
			  _function(f), _proj1(proj1), _proj2(proj2),
			  _out_iterator(_destination.ubegin()),
			  _in1_iterator(_source1.ubegin()), _in1_sentinel(_source1.uend()),
			  _in2_iterator(_source2.ubegin()), _in2_sentinel(_source2.uend())
		{}

		raze_always_inline constexpr void operator()(autovectorizable) requires(vectorizable()) {
			auto [in1_ptr, in1_last] = source1_type::to_raw_range(_in1_iterator, _in1_sentinel);
			auto [in2_ptr, in2_last] = source2_type::to_raw_range(_in2_iterator, _in2_sentinel);

			auto* raze_restrict out_ptr = std::to_address(_out_iterator);

			for (; in1_ptr != in1_last && in2_ptr != in2_last; ++in1_ptr, ++in2_ptr, ++out_ptr)
				*out_ptr = _function(_proj1(*in1_ptr), _proj2(*in2_ptr));

			source1_type::from_ptr(_in1_iterator, in1_ptr);
			source2_type::from_ptr(_in2_iterator, in2_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in1_iterator != _in1_sentinel && _in2_iterator != _in2_sentinel; ++_in1_iterator, ++_in2_iterator, ++_out_iterator)
				*_out_iterator = _function(_proj1(*_in1_iterator), _proj2(*_in2_iterator));
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* in1_ptr = std::to_address(_in1_iterator);
			auto* in2_ptr = std::to_address(_in2_iterator);
			auto* out_ptr = std::to_address(_out_iterator);
			const auto aligned_end = bytes_pointer_offset(in1_ptr, aligned_size);

			do {
				vx::store(out_ptr, _function(_proj1(vx::load<Tag>(in1_ptr)), _proj2(vx::load<Tag>(in2_ptr))));
				advance_bytes(in1_ptr, in2_ptr, sizeof(Tag));
				advance_bytes(out_ptr, sizeof(Tag));
			} while (in1_ptr != aligned_end);

			source1_type::from_ptr(_in1_iterator, in1_ptr);
			source2_type::from_ptr(_in2_iterator, in2_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			auto* in1_ptr = std::to_address(_in1_iterator);
			auto* in2_ptr = std::to_address(_in2_iterator);
			auto* out_ptr = std::to_address(_out_iterator);

			const auto data1 = vx::load<Tag>(in1_ptr);
			const auto data2 = vx::load<Tag>(in2_ptr);
			vx::store[ignore](out_ptr, _function(_proj1(data1), _proj2(data2)));

			advance_bytes(in1_ptr, in2_ptr, ignore.tail_bytes());
			advance_bytes(out_ptr, ignore.tail_bytes());

			source1_type::from_ptr(_in1_iterator, in1_ptr);
			source2_type::from_ptr(_in2_iterator, in2_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		constexpr raze_always_inline std::ranges::binary_transform_result<iterator1_type, iterator2_type, destination_iterator_type> result() const {
			return { _source1.wrap(_in1_iterator), _source2.wrap(_in2_iterator), _destination.wrap(_out_iterator) };
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() requires(constexpr_sized_source<Source1> && constexpr_sized_source<Source2>) {
			constexpr auto sz1 = Source1::static_size();
			constexpr auto sz2 = Source2::static_size();
			if constexpr (sizetype(sz1) < sizetype(sz2)) return sz1;
			else return sz2;
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return math::min(_source1.size(), _source2.size());
		}
	};

	template <class Source, class Destination, class Function, class Projection>
	kernel(Source&&, Destination&&, Function, Projection)
		-> kernel<std::remove_cvref_t<Source>, std::remove_cvref_t<Destination>, std::remove_cvref_t<Function>, std::remove_cvref_t<Projection>>;

	template <class Source1, class Source2, class Destination, class Function, class Projection1, class Projection2>
	kernel(Source1&&, Source2&&, Destination&&, Function, Projection1, Projection2)
		-> kernel<std::remove_cvref_t<Source1>, std::remove_cvref_t<Source2>, std::remove_cvref_t<Destination>, std::remove_cvref_t<Function>, std::remove_cvref_t<Projection1>, std::remove_cvref_t<Projection2>>;

	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent,
	    std::weakly_incrementable OutIt, class Function, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<InIt, OutIt> operator()(
		InIt first, Sent last, OutIt out, Function f, Proj proj = {}) const
			requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function, std::projected<InIt, Proj>>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			get_destination(std::move(out)), traits::fwd_fn(f), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt,
	    class Function, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<R>, OutIt>
		operator()(R&& r, OutIt out, Function f, Proj proj = {}) const
			requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function, 
				std::projected<std::ranges::iterator_t<R>, Proj>>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			get_destination(std::move(out)), traits::fwd_fn(f), traits::fwd_fn(proj));
	}

	template <std::input_iterator InIt1, std::sentinel_for<InIt1> Sent1,
		std::input_iterator InIt2, std::sentinel_for<InIt2> Sent2, std::weakly_incrementable OutIt, 
		class Function, class Proj1 = std::identity, class Proj2 = std::identity>
	constexpr raze_always_inline std::ranges::binary_transform_result<InIt1, InIt2, OutIt> operator()(
		InIt1 first1, Sent1 last1, InIt2 first2, Sent2 last2, OutIt out,
		Function f, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function,
				std::projected<InIt1, Proj1>, std::projected<InIt2, Proj2>>>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)), get_destination(std::move(out)),
			traits::fwd_fn(f), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2,
		std::weakly_incrementable OutIt, class Function, class Proj1 = std::identity,
		class Proj2 = std::identity>
	constexpr raze_always_inline std::ranges::binary_transform_result<
		std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>, OutIt> operator()(
		R1&& r1, R2&& r2, OutIt out, Function f, Proj1 proj1 = {}, Proj2 proj2 = {}) const
			requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function, 
				std::projected<std::ranges::iterator_t<R1>, Proj1>, std::projected<std::ranges::iterator_t<R2>, Proj2>>>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
			get_destination(std::move(out)), traits::fwd_fn(f), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
	}
};

/**
 * @brief Applies a given function to a range (or a pair of ranges) and stores the result in another range.
 *
 * 1) **Unary transform (iterator-sentinel)**: Applies the unary function `f` to each element in `[first, last)`
 *    projected by `proj`, and writes the results to the destination range beginning at `out`.
 *
 * 2) **Unary transform (range)**: Same as (1), but uses `r` as the input range, as if by:
 *    `transform(std::ranges::begin(r), std::ranges::end(r), std::move(out), f, proj)`.
 *
 * 3) **Binary transform (iterator-sentinel)**: Applies the binary function `f` to pairs of elements from
 *    `[first1, last1)` and `[first2, last2)` projected by `proj1` and `proj2` respectively, and writes the results
 *    to `out`. Evaluation stops when either input range reaches its end.
 *
 * 4) **Binary transform (range)**: Same as (3), but uses `r1` and `r2` as the input ranges, as if by:
 *    `transform(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), std::move(out), f, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Unary iterator-sentinel overload
 * template< std::input_iterator InIt, std::sentinel_for<InIt> Sent,
 *           std::weakly_incrementable OutIt, class Function, class Proj = std::identity >
 *   requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function, std::projected<InIt, Proj>>>)
 * constexpr std::ranges::unary_transform_result<InIt, OutIt>
 * transform( InIt first, Sent last, OutIt out, Function f, Proj proj = {} );
 *
 * // (2) Unary range overload
 * template< std::ranges::input_range R, std::weakly_incrementable OutIt,
 *           class Function, class Proj = std::identity >
 *   requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function, std::projected<std::ranges::iterator_t<R>, Proj>>>)
 * constexpr std::ranges::unary_transform_result<std::ranges::iterator_t<R>, OutIt>
 * transform( R&& r, OutIt out, Function f, Proj proj = {} );
 *
 * // (3) Binary iterator-sentinel overload
 * template< std::input_iterator InIt1, std::sentinel_for<InIt1> Sent1,
 *           std::input_iterator InIt2, std::sentinel_for<InIt2> Sent2,
 *           std::weakly_incrementable OutIt, class Function,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function,
 *            std::projected<InIt1, Proj1>, std::projected<InIt2, Proj2>>>)
 * constexpr std::ranges::binary_transform_result<InIt1, InIt2, OutIt>
 * transform( InIt1 first1, Sent1 last1, InIt2 first2, Sent2 last2, OutIt out,
 *            Function f, Proj1 proj1 = {}, Proj2 proj2 = {} );
 *
 * // (4) Binary range overload
 * template< std::ranges::input_range R1, std::ranges::input_range R2,
 *           std::weakly_incrementable OutIt, class Function,
 *           class Proj1 = std::identity, class Proj2 = std::identity >
 *   requires(std::indirectly_writable<OutIt, std::indirect_result_t<Function,
 *            std::projected<std::ranges::iterator_t<R1>, Proj1>, std::projected<std::ranges::iterator_t<R2>, Proj2>>>)
 * constexpr std::ranges::binary_transform_result<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>, OutIt>
 * transform( R1&& r1, R2&& r2, OutIt out,
 *            Function f, Proj1 proj1 = {}, Proj2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last`   - the range of elements to transform (unary)
 * - `r`               - the range of elements to transform (unary)
 * - `first1`, `last1` - the first range of elements to transform (binary)
 * - `first2`, `last2` - the second range of elements to transform (binary)
 * - `r1`              - the first range of elements to transform (binary)
 * - `r2`              - the second range of elements to transform (binary)
 * - `out`             - the beginning of the destination range
 * - `f`               - transformation function (unary or binary)
 * - `proj`, `proj1`, `proj2` - projections to apply to input elements (default to `std::identity`)
 *
 * ### Return value
 * - For **unary overloads** (1, 2): A `std::ranges::unary_transform_result` containing:
 *   - `in`: an iterator pointing to the end of the input range (`last`).
 *   - `out`: an iterator pointing past the last written element in the destination range.
 * - For **binary overloads** (3, 4): A `std::ranges::binary_transform_result` containing:
 *   - `in1`: an iterator pointing past the last consumed element in the first range.
 *   - `in2`: an iterator pointing past the last consumed element in the second range.
 *   - `out`: an iterator pointing past the last written element in the destination range.
 *
 * ### Complexity
 * - Unary: Exactly `last - first` (or `std::ranges::distance(r)`) invocations of `f` and `proj`.
 * - Binary: Exactly `min(last1 - first1, last2 - first2)` (or `min(distance(r1), distance(r2))`) invocations of `f`, `proj1`, and `proj2`.
 *
 * ### Decorators and Options
 * The algorithm object supports compile-time modifiers via `operator[]`:
 * ```cpp
 * transform[raze::options::fscalar](...);
 * transform[raze::options::unroll<2>](...);
 * transform[raze::options::fstatic][raze::options::unroll<2>](...);
 * ```
 * For details on available options (`fscalar`, `fstatic`, `unroll`), their semantics,
 * and valid combinations, see the **options documentation**.
 *
 * ### Notes
 * - Unlike `std::ranges::transform` for binary operations, this algorithm safely processes
 *   ranges of mismatched lengths up to the length of the shorter range.
 * - If sources and destinations model contiguous buffers, and user-provided functions/projections
 *   satisfy vectorization requirements, explicit SIMD instructions (AVX-512, AVX2, SSE)
 *   or platform-specific autovectorization paths are utilized.
 * - In a constant-evaluated context, the algorithm executes via the scalar fallback path.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <functional>
 * #include <raze/algorithm/transform/Transform.h>
 *
 * void println(const auto& seq) {
 *     for (const auto& elem : seq) std::cout << elem << ' ';
 *     std::cout << '\n';
 * }
 *
 * int main() {
 *     const std::vector<int> a{1, 2, 3, 4, 5};
 *     const std::vector<int> b{10, 20, 30, 40, 50, 60, 70};
 *     std::vector<int> res(a.size());
 *
 *     // Unary transform: square each element of 'a'
 *     raze::algorithm::transform(a, res.begin(), [](int x) { return x * x; });
 *     println(res);
 *
 *     // Binary transform: pairwise addition of 'a' and 'b' up to min(a.size(), b.size())
 *     raze::algorithm::transform(a, b, res.begin(), std::plus<>{});
 *     println(res);
 * }
 * ```
 *
 * Possible output:
 * ```text
 * 1 4 9 16 25 
 * 11 22 33 44 55 
 * ```
 */
constexpr inline auto transform = options::function_with_traits<transform_t>[options::unroll<4>][transform_strategy];

__RAZE_ALGORITHM_NAMESPACE_END