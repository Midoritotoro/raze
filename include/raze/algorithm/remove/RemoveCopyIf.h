#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/Destination.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto remove_copy_strategy = options::strategy<strategy<>{}>;

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

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> && contiguous_destination<Destination> &&
				vectorizable_unary_predicate<Predicate, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		source_type _source;
		destination_type _dest;
		unchecked_out_iterator_type _out_it;
		unchecked_iterator_type _in_it;
		unchecked_sentinel_type _in_sent;
		Predicate _predicate;
		Projection _proj;

		constexpr explicit kernel(Source&& source, Destination&& dest, Predicate pred, Projection proj)
			: _source(std::forward<Source>(source)), _dest(std::forward<Destination>(dest)), 
				_predicate(pred), _proj(proj), _out_it(_source.ubegin()), _in_it(_source.ubegin()),
			  _in_sent(_source.uend())
		{}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in_it != _in_sent; ++_in_it) {
				if (!_predicate(_proj(*_in_it))) {
					*_out_it = std::ranges::iter_move(_in_it);
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
			source_type::from_ptr(_out_it, out_ptr);
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

	template <class Source, class Destination, class Predicate, class Projection>
	kernel(Source&&, Destination&&, Predicate, Projection)
		-> kernel<std::remove_cvref_t<Source>, std::remove_cvref_t<Destination>, std::remove_cvref_t<Predicate>, std::remove_cvref_t<Projection>>;

	template <std::permutable It, std::sentinel_for<It> Sent, class OutIt, class Pred, class Proj = std::identity>
	constexpr raze_always_inline decltype(auto) operator()(
		It first, Sent last, OutIt out, Pred pred, Proj proj = {}) const
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), get_destination(std::move(out)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class OutIt, class Pred, class Proj = std::identity>
	constexpr raze_always_inline decltype(auto) operator()(
		R&& r, OutIt out, Pred pred, Proj proj = {}) const
	{
		return this->dispatch(get_source(std::forward<R>(r)), get_destination(std::move(out)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

constexpr inline auto remove_copy_if = options::function_with_traits<remove_copy_if_t>[remove_copy_strategy];

__RAZE_ALGORITHM_NAMESPACE_END