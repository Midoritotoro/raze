#pragma once 

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto replace_copy_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct replace_copy_if_t : Traits, dispatchable<replace_copy_if_t<Traits>> {
	template <source Source, destination Destination, class Predicate, class Projection, class T>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using destination_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_destination_type = typename destination_type::unchecked_iterator_type;

		using vector_value_type = std::iter_value_t<iterator_type>;

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				contiguous_destination<Destination> && 
				vectorizable_unary_predicate<Predicate, unchecked_iterator_type>&&
				vectorizable_projection<Projection, unchecked_iterator_type>;
		}

		destination_type _destination;
		source_type _source;
		unchecked_iterator_type _in_iterator;
		unchecked_sentinel_type _in_sentinel;
		unchecked_destination_type _out_iterator;
		Predicate _predicate;
		Projection _proj;
		T _new_value;

		constexpr explicit kernel(Source&& source, Destination&& dest,
			Predicate pred, Projection proj, const T& new_val):
				_source(std::forward<Source>(source)), _destination(std::forward<Destination>(dest)),
				_predicate(pred), _proj(proj), _new_value(new_val),  
				_out_iterator(_destination.ubegin()), _in_iterator(_source.ubegin()), _in_sentinel(_source.uend())
		{}

		raze_always_inline constexpr void operator()(autovectorizable) requires(vectorizable()) {
			auto* raze_restrict first = std::to_address(_in_iterator);
			auto* raze_restrict last = std::to_address(_in_sentinel);
			auto* raze_restrict dest = std::to_address(_out_iterator);

			for (; first != last; ++first, ++dest)
				*dest = _predicate(_proj(*first)) ? _new_value : *first;
		}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for (; _in_iterator != _in_sentinel; ++_in_iterator, ++_out_iterator)
				*_out_iterator = _predicate(_proj(*_in_iterator)) ? _new_value : *_in_iterator;
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);

			const auto aligned_end = bytes_pointer_offset(in_ptr, aligned_size);

			do {
				const auto data = vx::load<Tag>(in_ptr);
				vx::store(out_ptr, vx::select[_predicate(_proj(data)), data](Tag(_new_value)));
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
			vx::store[ignore](out_ptr, vx::select[_predicate(_proj(data)), data](Tag(_new_value)));

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

	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent,
		std::weakly_incrementable OutIt, class Pred, class T, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<InIt, OutIt> operator()(
		InIt first, Sent sent, OutIt out, Pred pred, const T& v, Proj proj = {}) const
			requires(std::indirectly_copyable<InIt, OutIt> &&
				std::indirect_unary_predicate<Pred, std::projected<InIt, Proj>> &&
				std::indirectly_writable<OutIt, const T&>)
	{
		return this->dispatch(get_source(std::move(first), std::move(sent)),
			get_destination(std::move(out)), traits::fwd_fn(pred), traits::fwd_fn(proj), v);
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt, class Pred, class T, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<R>, OutIt> operator()(
		R&& r, OutIt out, Pred pred, const T& v, Proj proj = {}) const
			requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> &&
				std::indirect_unary_predicate<Pred, std::projected<std::ranges::iterator_t<R>, Proj>> &&
				std::indirectly_writable<OutIt, const T&>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), get_destination(std::move(out)),
			traits::fwd_fn(pred), traits::fwd_fn(proj), v);
	}
};

constexpr inline auto replace_copy_if = options::function_with_traits<replace_copy_if_t>[options::unroll<4>][replace_copy_strategy];

__RAZE_ALGORITHM_NAMESPACE_END