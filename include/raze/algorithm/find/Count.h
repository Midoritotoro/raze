#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Count_if : _Traits_, dispatchable<_Count_if<_Traits_>> {
	template <class _DiffType_, source _Source_, class Predicate, class Projection>
	struct __kernel {
		using source_type = std::remove_cvref_t<_Source_>;
		using iterator_type = typename source_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<iterator_type>;

		_Source_ _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		Predicate _predicate;
		Projection _proj;
		_DiffType_ _count = 0;

		constexpr explicit __kernel(options::as<_DiffType_>, _Source_&& __src, Predicate __pred, Projection __proj) noexcept :
			_source(std::forward<_Source_>(__src)), _predicate(__pred), _proj(__proj)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}

		raze_always_inline constexpr void operator()() noexcept {
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator)
				_count += _predicate(_proj(*_iterator));
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline void operator()(_Tag_, sizetype __aligned_size) noexcept {
			vx::counter<_Tag_> __counter;
				
			auto* __ptr = std::to_address(_iterator);
			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			while (__ptr != __aligned_end) {
				auto __current_portion_size = __counter.portion_size();

				raze_disable_unrolling
				while (__ptr != __aligned_end && __current_portion_size > 0) {
					__counter.count(_predicate(_proj(vx::load<_Tag_>(__ptr))));
					__advance_bytes(__ptr, sizeof(_Tag_));
					--__current_portion_size;
				}

				_count += __counter.result();
			}

			source_type::from_ptr(_iterator, __ptr);
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline void operator()(_Tag_, tail_mask_type auto const& __ignore) noexcept {
			_count += vx::count_set[__ignore](_predicate(_proj(vx::load<_Tag_>[__ignore](std::to_address(_iterator)))));
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept requires(constexpr_sized_source<_Source_>) {
			return _Source_::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const noexcept {
			return _source.size();
		}

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				vectorizable_unary_predicate<Predicate, unchecked_iterator_type>&&
				vectorizable_projection<Projection, unchecked_iterator_type>;
		}

		constexpr raze_always_inline _DiffType_ result() const noexcept {
			return _count;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class Predicate, class Projection = std::identity>
	raze_nodiscard constexpr raze_always_inline std::iter_difference_t<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __sent, Predicate __pred, Projection __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<Predicate, std::projected<_Iterator_, Projection>>)
	{
		return __raze_kernel_dispatch_call(options::as(std::iter_difference_t<_Iterator_>{}),
			get_source(std::move(__first), std::move(__sent)), traits::fwd_fn(__pred), traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<Range> operator()(Range&& __r, 
		Predicate __pred, Projection __proj = {}) const noexcept requires(std::indirect_unary_predicate<
			Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>>)
	{
		return __raze_kernel_dispatch_call(options::as(std::ranges::range_difference_t<Range>{}),
			get_source(std::forward<Range>(__r)), traits::fwd_fn(__pred), traits::fwd_fn(__proj));
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto count_if = raze::options::function_with_traits<_Count_if>[options::unroll<4>];

template <class _Traits_>
struct _Count : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_, class Projection = std::identity>
	raze_nodiscard constexpr raze_always_inline std::iter_difference_t<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, Projection __proj = {}) const noexcept
	{
		return count_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<Projection, std::iter_value_t<_Iterator_>>(__v)),
			traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, class _Value_, class Projection = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<Range> operator()(
		Range&& __range, const _Value_& __v, Projection __proj = {}) const noexcept
	{
		return count_if[_Traits_::traits()](std::forward<Range>(__range), algorithm::equal_to(
			function_return_type<Projection, std::ranges::range_value_t<Range>>(__v)),
			traits::fwd_fn(__proj));
	}
};

constexpr inline auto count = raze::options::function_with_traits<_Count>[options::unroll<4>];

template <class _Traits_>
struct _Count_if_not : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class Predicate, class Projection = std::identity>
	raze_nodiscard constexpr raze_always_inline std::iter_difference_t<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, Predicate __pred, Projection __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<Predicate, std::projected<_Iterator_, Projection>>)
	{
		return count_if[_Traits_::traits()](std::move(__first), std::move(__last), make_not_fn(__pred), traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, class Predicate, class Projection = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<Range> operator()(
		Range&& __range, Predicate __pred, Projection __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>>)
	{
		return count_if[_Traits_::traits()](std::forward<Range>(__range), make_not_fn(__pred), traits::fwd_fn(__proj));
	}
};

constexpr inline auto count_if_not = raze::options::function_with_traits<_Count_if_not>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
