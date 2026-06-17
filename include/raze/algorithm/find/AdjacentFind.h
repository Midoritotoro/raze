#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Adjacent_find : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;

			auto __next = _iterator;
			++__next;

			if (__next == _sentinel) {
				++_iterator;
				return true;
			}
			else if (_predicate(_proj(*_iterator), _proj(*__next))) return true;
			else {
				++_iterator;
				return false;
			}
		}

		template <vx::simd_type _Tag_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, sizetype __aligned_size, sizetype __tail_size) noexcept {
			auto* __ptr = std::to_address(_iterator);
			auto* __end = __bytes_pointer_offset(__ptr, __aligned_size + __tail_size);

			raze_assume(__ptr != __nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);
			auto __next_ptr = __bytes_pointer_offset(__ptr, sizeof(typename _Tag_::value_type));

			do {
				const auto __current = _proj(raze::vx::load<_Tag_>(__ptr));
				const auto __next = _proj(raze::vx::load<_Tag_>(__next_ptr));
				
				const auto __mask = _predicate(__current, __next);

				if (raze::vx::any_of(__mask)) {
					__seek_possibly_wrapped_iterator(_iterator, __ptr + raze::vx::find_first_set(__mask));
					return;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
				__advance_bytes(__next_ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(_iterator, __ptr);

			if (_iterator == _sentinel)
				return;

			for (auto __next = _iterator; ++__next != _sentinel; _iterator = __next)
				if (_predicate(_proj(*_iterator), _proj(*__next)))
					break;
		}

		template <vx::simd_type _Tag_, sizetype _AlignedSize_, sizetype _TailSize_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, 
			std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>) noexcept 
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(_iterator);
			auto* __end = __bytes_pointer_offset(__ptr, _AlignedSize_ + _TailSize_);

			raze_assume(__ptr != nullptr);

			auto __next_ptr = __bytes_pointer_offset(__ptr, sizeof(typename _Tag_::value_type));
			auto __left = __iterations_aligned;

			do {
				const auto __current = _proj(raze::vx::load<_Tag_>(__ptr));
				const auto __next = _proj(raze::vx::load<_Tag_>(__next_ptr));
				
				const auto __mask = _predicate(__current, __next);

				if (raze::vx::any_of(__mask)) {
					__seek_possibly_wrapped_iterator(_iterator, __ptr + raze::vx::find_first_set(__mask));
					return;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
				__advance_bytes(__next_ptr, sizeof(_Tag_));
			} while (--__left);
			
			__seek_possibly_wrapped_iterator(_iterator, __ptr);

			if constexpr (_TailSize_ == 0)
				return;

			for (auto __next = _iterator; ++__next != _sentinel; _iterator = __next)
				if (_predicate(_proj(*_iterator), _proj(*__next)))
					break;
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred = {}, _Projection_ __proj = {}) const noexcept
	{
		__seek_possibly_wrapped_iterator(__first, __adjacent_find_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj)));

		return __first;
	}

	template <std::ranges::input_range _Range_, class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred = {}, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		return __adjacent_find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred = {}, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		return __adjacent_find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj), 
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __adjacent_find_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj) const noexcept
	{
		using _Value_ = std::iter_value_t<_Iterator_>;

		__verify_range(__first, __last);
		auto __work = __impl(__first, __last, __pred, __proj);
		
		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				__seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl,
					_Value_, _Iterator_>(algorithm::distance(__first, __last) * sizeof(_Value_), __work));
				return __first;
			}
		}
		
		__seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work));
		return __first;
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __adjacent_find_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		using _Value_ = std::iter_value_t<_Iterator_>;

		__verify_range(__first, __last);
		auto __work = __impl(__first, __last, __pred, __proj);

		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				__seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl,
					_Value_, _Iterator_>(std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{}, __work));
				return __first;
			}
		}
		
		__seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work));
		return __first;
	}
};

constexpr inline auto adjacent_find = raze::options::function_with_traits<_Adjacent_find>;

__RAZE_ALGORITHM_NAMESPACE_END
