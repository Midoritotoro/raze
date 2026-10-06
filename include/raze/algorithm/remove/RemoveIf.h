#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct remove_if_t : Traits, dispatchable<remove_if_t<Traits>, arch::ISA::AVX512VBMI2, arch::ISA::AVX2, arch::ISA::SSSE3> {
	template <source Source, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
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
			: _source(std::forward<Source>(source)), _predicate(pred), _proj(proj),
			  _write_iterator(_source.ubegin()), _in_iterator(_source.ubegin()),
			  _in_sentinel(_source.uend())
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
	};

	template <class Source, class Predicate, class Projection>
	kernel(Source&&, Predicate, Projection)
		-> kernel<std::remove_cvref_t<Source>, std::remove_cvref_t<Predicate>, std::remove_cvref_t<Projection>>;

	template <std::permutable It, std::sentinel_for<It> Sent, class Pred, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It> operator()(
		It first, Sent last, Pred pred, Proj proj = {}) const
			requires(std::indirect_unary_predicate<Pred, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class Pred, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R> operator()(
		R&& r, Pred pred, Proj proj = {}) const
			requires(std::permutable<std::ranges::iterator_t<R>> &&
				std::indirect_unary_predicate<Pred, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

constexpr inline auto remove_if = options::function_with_traits<remove_if_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END