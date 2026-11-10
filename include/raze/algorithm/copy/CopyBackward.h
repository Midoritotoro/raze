#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/memory/Memmove.h>
#include <raze/options/Options.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct copy_backward_t : Traits, dispatchable<copy_backward_t<Traits>> {
	template <source Source, destination Destination>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using destination_type = std::remove_cvref_t<Destination>;

		using iterator_type = typename source_type::iterator_type;
		using out_iterator_type = typename destination_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;

		using vt_in = std::iter_value_t<unchecked_iterator_type>;
		using vt_out = std::iter_value_t<unchecked_out_iterator_type>;

		using vector_value_type = vt_in;

		static consteval bool vectorizable() noexcept { 
			return contiguous_source<source_type> && contiguous_destination<destination_type> &&
				vx::arithmetic_type<vt_in> && vx::arithmetic_type<vt_out>;
		}

		static consteval bool native() noexcept {
			return contiguous_source<source_type> && contiguous_destination<destination_type> &&
				std::is_trivially_copyable_v<vt_in> && std::same_as<vt_in, vt_out>;
		}

		source_type _source;
		destination_type _dest;
		unchecked_iterator_type _in_iterator;
		unchecked_iterator_type _in_end;
		unchecked_iterator_type _in_start_ret;
		unchecked_out_iterator_type _out_iterator;

		constexpr explicit kernel(Source&& source, Destination&& dest)
			: _source(std::forward<Source>(source)),
			  _dest(std::forward<Destination>(dest)),
			  _in_iterator(_source.ubegin()),
			  _out_iterator(_dest.ubegin()),
			  _in_end(std::ranges::next(_in_iterator, _source.uend())),
			  _in_start_ret(_in_end)
		{}

		raze_always_inline constexpr void operator()() {
			raze_disable_unrolling
			for ( ; _in_iterator != _in_end; )
				*--_out_iterator = *--_in_end;
		}

		raze_always_inline void operator()(native_tag) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);
			rewind_bytes(out_ptr, size());
			__raze_memmove(out_ptr, in_ptr, size());
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) requires(vx::supports_fast_conversion<Tag, vt_out>) {
			auto* in_end_ptr = std::to_address(_in_end);
			auto* out_ptr = std::to_address(_out_iterator);

			auto aligned_begin = std::to_address(in_end_ptr);
			rewind_bytes(aligned_begin, aligned_size);

			raze_disable_unrolling do {
				rewind_bytes(in_end_ptr, sizeof(Tag));
				rewind_bytes(out_ptr, Tag::size() * sizeof(vt_out));
				vx::store(out_ptr, vx::convert<vt_out>(vx::load<Tag>(in_end_ptr)));
			} while (in_end_ptr != aligned_begin);

			source_type::from_ptr(_in_end, in_end_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) requires(std::same_as<vt_in, vt_out>) {
			auto* in_ptr = std::to_address(_in_end);
			auto* out_ptr = std::to_address(_out_iterator); 
			rewind_bytes(out_ptr, ignore.tail_bytes()); 
			vx::store[ignore](out_ptr, vx::load<Tag>[ignore](in_ptr));
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		constexpr raze_always_inline std::ranges::in_out_result<iterator_type, out_iterator_type> result() const {
			return { _source.wrap(_in_start_ret), _dest.wrap(_out_iterator) };
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
			requires(constexpr_sized_source<source_type>)
		{
			return source_type::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <std::bidirectional_iterator InIt, std::sentinel_for<InIt> Sent, std::weakly_incrementable OutIt>
	constexpr raze_always_inline std::ranges::in_out_result<InIt, OutIt> operator()(InIt first, 
		Sent last, OutIt result) const requires(std::indirectly_copyable<InIt, OutIt>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), get_destination(std::move(result)));
	}

	template <std::ranges::bidirectional_range R, std::weakly_incrementable OutIt>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<R>, OutIt>
		operator()(R&& r, OutIt result) const requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), get_destination(std::move(result)));
	}
};

constexpr inline auto copy_backward = options::function_with_traits<copy_backward_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END