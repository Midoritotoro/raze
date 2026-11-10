#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/memory/Memcpy.h>
#include <raze/options/Options.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct copy_t : Traits, dispatchable<copy_t<Traits>> {
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
		unchecked_sentinel_type _in_sentinel;
		unchecked_out_iterator_type _out_iterator;

		constexpr explicit kernel(Source&& source, Destination&& dest)
			: _source(std::forward<Source>(source)),
			  _dest(std::forward<Destination>(dest)),
			  _in_iterator(_source.ubegin()), _in_sentinel(_source.uend()),
			  _out_iterator(_dest.ubegin())
		{}

		raze_always_inline constexpr void operator()() {
			for_novec(_in_iterator, _in_sentinel, _out_iterator,
				[] (auto&& value) -> decltype(auto) { return std::forward<decltype(value)>(value); });
		}

		raze_always_inline void operator()(native_tag) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);

			auto out_end = __raze_memcpy(out_ptr, in_ptr, size());

			source_type::from_ptr(_in_iterator, bytes_pointer_offset(in_ptr, size()));
			destination_type::from_ptr(_out_iterator, static_cast<decltype(out_ptr)>(out_end));
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) requires(vx::supports_fast_conversion<Tag, vt_out>) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);

			const auto aligned_end = bytes_pointer_offset(in_ptr, aligned_size);

			raze_disable_unrolling do {
				vx::store(out_ptr, vx::convert<vt_out>(vx::load<Tag>(in_ptr)));
				advance_bytes(in_ptr, sizeof(Tag));
				advance_bytes(out_ptr, Tag::size() * sizeof(vt_out));
			} while (in_ptr != aligned_end);

			source_type::from_ptr(_in_iterator, in_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) requires(std::same_as<vt_in, vt_out>) {
			auto* in_ptr = std::to_address(_in_iterator);
			auto* out_ptr = std::to_address(_out_iterator);

			vx::store[ignore](out_ptr, vx::load<Tag>[ignore](in_ptr));
			advance_bytes(in_ptr, out_ptr, ignore.tail_bytes());

			source_type::from_ptr(_in_iterator, in_ptr);
			destination_type::from_ptr(_out_iterator, out_ptr);
		}

		constexpr raze_always_inline std::ranges::in_out_result<iterator_type, out_iterator_type> result() const {
			return { _source.wrap(_in_iterator), _dest.wrap(_out_iterator) };
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

	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent, std::weakly_incrementable OutIt>
	constexpr raze_always_inline std::ranges::in_out_result<InIt, OutIt> operator()(InIt first, 
		Sent last, OutIt result) const requires(std::indirectly_copyable<InIt, OutIt>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), get_destination(std::move(result)));
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<R>, OutIt>
		operator()(R&& r, OutIt result) const requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), get_destination(std::move(result)));
	}
};

constexpr inline auto copy = options::function_with_traits<copy_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END