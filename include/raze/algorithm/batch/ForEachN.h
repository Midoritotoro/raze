#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto for_each_n_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct for_each_n_t : Traits, dispatchable<for_each_n_t<Traits>> {
	template <source Source, class F, class Proj>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using vector_value_type = std::iter_value_t<iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
				vectorizable_unary_function<F, unchecked_iterator_type> &&
				vectorizable_projection<Proj, unchecked_iterator_type>;
		}

		source_type _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		F _f;
		Proj _proj;

		constexpr explicit kernel(Source&& src, F f, Proj proj):
			_source(std::forward<Source>(src)), _iterator(_source.ubegin()),
			_sentinel(_source.uend()), _f(f), _proj(proj)
		{}

		raze_always_inline constexpr void operator()(autovectorizable) requires(vectorizable()) {
			auto* raze_restrict first = std::to_address(_iterator);
			auto* raze_restrict last = std::to_address(_sentinel);

			for (; first != last; ++first)
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

		constexpr raze_always_inline std::ranges::in_fun_result<iterator_type, F> result() const {
			return { _source.wrap(_iterator).base(), _f };
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <std::input_iterator InIt, class F, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::in_fun_result<InIt, F> operator()(
		InIt first, std::iter_difference_t<InIt> n, F f, Proj proj = {}) const
			requires(std::indirectly_unary_invocable<F, std::projected<InIt, Proj>>)
	{
		return this->dispatch(get_source(std::counted_iterator<InIt>(std::move(first), n), 
			std::default_sentinel), traits::fwd_fn(f), traits::fwd_fn(proj));
	}
};

constexpr inline auto for_each_n = options::function_with_traits<for_each_n_t>[options::unroll<4>][for_each_n_strategy];

__RAZE_ALGORITHM_NAMESPACE_END