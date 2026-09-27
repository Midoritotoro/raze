#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto swap_ranges_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct swap_ranges_t : Traits, dispatchable<swap_ranges_t<Traits>> {
	template <source Source1, source Source2>
	struct kernel {
		using source1_type = std::remove_cvref_t<Source1>;
		using source2_type = std::remove_cvref_t<Source2>;

		using iterator1_type = typename source1_type::iterator_type;
		using iterator2_type = typename source2_type::iterator_type;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;
		using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator1_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source1> && contiguous_source<Source2> &&
				std::is_trivially_copyable_v<vector_value_type> &&
				std::same_as<vector_value_type, std::iter_value_t<unchecked_iterator2_type>>;
		}

		source1_type _source1;
		source2_type _source2;
		unchecked_iterator1_type _in1_iterator;
		unchecked_sentinel1_type _in1_sentinel;
		unchecked_iterator2_type _in2_iterator;
		unchecked_sentinel2_type _in2_sentinel;

		constexpr explicit kernel(Source1&& source1, Source2&& source2)
			: _source1(std::forward<Source1>(source1)),
			  _source2(std::forward<Source2>(source2)),
			  _in1_iterator(_source1.ubegin()), _in1_sentinel(_source1.uend()),
			  _in2_iterator(_source2.ubegin()), _in2_sentinel(_source2.uend())
		{}

		raze_always_inline constexpr void operator()(autovectorizable) requires(vectorizable()) {
			auto* raze_restrict in1_ptr = std::to_address(_in1_iterator);
			auto* raze_restrict in1_last = std::to_address(_in1_sentinel);
			auto* raze_restrict in2_ptr = std::to_address(_in2_iterator);
			auto* raze_restrict in2_last = std::to_address(_in2_sentinel);

			for (; in1_ptr != in1_last && in2_ptr != in2_last; ++in1_ptr, ++in2_ptr) {
				auto tmp = *in1_ptr;
				*in1_ptr = *in2_ptr;
				*in2_ptr = tmp;
			}

			source1_type::from_ptr(_in1_iterator, in1_ptr);
			source2_type::from_ptr(_in2_iterator, in2_ptr);
		}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in1_iterator != _in1_sentinel && _in2_iterator != _in2_sentinel; ++_in1_iterator, ++_in2_iterator)
				std::ranges::iter_swap(_in1_iterator, _in2_iterator);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* in1_ptr = std::to_address(_in1_iterator);
			auto* in2_ptr = std::to_address(_in2_iterator);
			const auto aligned_end = bytes_pointer_offset(in1_ptr, aligned_size);

			do {
				const auto v1 = vx::load<Tag>(in1_ptr);
				const auto v2 = vx::load<Tag>(in2_ptr);
				vx::store(in1_ptr, v2);
				vx::store(in2_ptr, v1);
				advance_bytes(in1_ptr, in2_ptr, sizeof(Tag));
			} while (in1_ptr != aligned_end);

			source1_type::from_ptr(_in1_iterator, in1_ptr);
			source2_type::from_ptr(_in2_iterator, in2_ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
			auto* in1_ptr = std::to_address(_in1_iterator);
			auto* in2_ptr = std::to_address(_in2_iterator);

			const auto v1 = vx::load<Tag>(in1_ptr);
			const auto v2 = vx::load<Tag>(in2_ptr);
			vx::store[ignore](in1_ptr, v2);
			vx::store[ignore](in2_ptr, v1);

			advance_bytes(in1_ptr, in2_ptr, ignore.tail_bytes());

			source1_type::from_ptr(_in1_iterator, in1_ptr);
			source2_type::from_ptr(_in2_iterator, in2_ptr);
		}

		constexpr raze_always_inline std::ranges::swap_ranges_result<iterator1_type, iterator2_type> result() const {
			return { _source1.wrap(_in1_iterator), _source2.wrap(_in2_iterator) };
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

	template <class Source1, class Source2>
	kernel(Source1&&, Source2&&) -> kernel<std::remove_cvref_t<Source1>, std::remove_cvref_t<Source2>>;

	template <std::input_iterator InIt1, std::sentinel_for<InIt1> Sent1,
		std::input_iterator InIt2, std::sentinel_for<InIt2> Sent2>
	constexpr raze_always_inline std::ranges::swap_ranges_result<InIt1, InIt2> operator()(
		InIt1 first1, Sent1 last1, InIt2 first2, Sent2 last2) const
			requires(std::indirectly_swappable<InIt1, InIt2>)
	{
		return this->dispatch(get_source(std::move(first1), std::move(last1)),
			get_source(std::move(first2), std::move(last2)));
	}

	template <std::ranges::input_range R1, std::ranges::input_range R2>
	constexpr raze_always_inline std::ranges::swap_ranges_result<
		std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>> operator()(R1&& r1, R2&& r2) const
			requires(std::indirectly_swappable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>>)
	{
		return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)));
	}
};

constexpr inline auto swap_ranges = options::function_with_traits<swap_ranges_t>[options::unroll<4>][swap_ranges_strategy];

__RAZE_ALGORITHM_NAMESPACE_END