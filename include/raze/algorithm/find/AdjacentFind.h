#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Adjacent_find : _Traits_ {
	template <source _Source_, class _Predicate_, class _Projection_>
	struct __kernel {
		using source_type = std::remove_cvref_t<_Source_>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		_Source_ _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		unchecked_iterator_type _next;
		_Predicate_ _predicate;
		_Projection_ _proj;

		constexpr explicit __kernel(_Source_&& __source, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_source(std::forward<_Source_>(__source)), _predicate(__pred), _proj(__proj)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}

		template <scalar_tag _Tag_>
		raze_always_inline constexpr void operator()(_Tag_) noexcept {
			if (_iterator == _sentinel)
				return;

			_next = _iterator;

			raze_disable_unrolling
			for (; ++_next != _sentinel; _iterator = _next)
				if (_predicate(_proj(*_iterator), _proj(*_next)))
					return;

			_iterator = _next;
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_, sizetype __aligned_size) noexcept {
			auto* __ptr = std::to_address(_iterator);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);
			auto __next_ptr = __bytes_pointer_offset(__ptr, sizeof(typename _Tag_::value_type));

			raze_disable_unrolling
			do {
				const auto __current = _proj(vx::load<_Tag_>(__ptr));
				const auto __next = _proj(vx::load<_Tag_>(__next_ptr));

				const auto __mask = _predicate(__current, __next);

				if (vx::any_of(__mask)) {
					source_type::from_ptr(_iterator, __ptr + vx::find_first_set[vx::not_null](__mask));
					return false;
				}

				__advance_bytes(__ptr, __next_ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			source_type::from_ptr(_iterator, __ptr);
			return true;
		}
		
		raze_nodiscard constexpr raze_always_inline auto size() const noexcept {
			return _source.size() - sizeof(std::iter_value_t<unchecked_iterator_type>);
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept requires(constexpr_sized_source<_Source_>) {	
			return std::integral_constant<sizetype, source_type::static_size() - sizeof(std::iter_value_t<unchecked_iterator_type>)>{};
		}

		raze_nodiscard constexpr raze_always_inline iterator_type result() const noexcept {
			return _source.wrap(_iterator);
		}

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				vectorizable_binary_predicate<_Predicate_, unchecked_iterator_type> &&
				vectorizable_projection<_Projection_, unchecked_iterator_type>;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred = {}, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_binary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>,
				std::projected<_Iterator_, _Projection_>>)
	{
		if (__first == __last) return __first;
		return __raze_kernel_dispatch_call(get_source(std::move(__first),
			std::move(__last)), traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __r, _Predicate_ __pred = {}, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_binary_predicate<_Predicate_, 
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>,
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		if (std::ranges::begin(__r) == std::ranges::end(__r)) return std::ranges::begin(__r);
		return __raze_kernel_dispatch_call(get_source(std::forward<_Range_>(__r)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto adjacent_find = raze::options::function_with_traits<_Adjacent_find>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
