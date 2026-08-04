#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Adjacent_find : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __impl {
		static constexpr auto can_enable_vectorization = std::contiguous_iterator<_Iterator_> && 
			vectorizable_binary_predicate<_Predicate_, _Iterator_> && vectorizable_projection<_Projection_, _Iterator_>;

		_Iterator_ _iterator;
		_Iterator_ _next;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

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
				const auto __current = _proj(raze::vx::load<_Tag_>(__ptr));
				const auto __next = _proj(raze::vx::load<_Tag_>(__next_ptr));

				const auto __mask = _predicate(__current, __next);

				if (raze::vx::any_of(__mask)) {
					__seek_iter(_iterator, __ptr + raze::vx::find_first_set[vx::not_null](__mask));
					return false;
				}

				__advance_bytes(__ptr, __next_ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_iter(_iterator, __ptr);
			return true;
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
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
		auto __size = __bytes_distance(__first, __last);

		__seek_iter(__first, __raze_kernel_dispatch_call(__size, traits::__uiter<_Sentinel_>(std::move(__first)),
			traits::__usent<_Iterator_>(std::move(__last)), traits::__fwd_fn(__pred), traits::__fwd_fn(__proj)));
		return __first;
	}

	template <std::ranges::input_range _Range_, class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __r, _Predicate_ __pred = {}, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_binary_predicate<_Predicate_, 
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>,
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		auto __first = std::ranges::begin(__r);
		auto __last = std::ranges::end(__r);
		if (__first == __last) return __first;

		__seek_iter(__first, __raze_kernel_dispatch_call(__bytes_distance(__r), traits::__r_uiter<_Range_>(std::move(__first)),
			traits::__r_usent<_Range_>(std::move(__last)), traits::__fwd_fn(__pred), traits::__fwd_fn(__proj)));
		return __first;
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto adjacent_find = raze::options::function_with_traits<_Adjacent_find>;

__RAZE_ALGORITHM_NAMESPACE_END
