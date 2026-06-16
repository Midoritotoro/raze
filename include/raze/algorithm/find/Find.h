#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find_if : _Traits_ {
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
			else if (_predicate(_proj(*_iterator))) return true;
			else {
				++_iterator;
				return false;
			}
		}

		template <vx::simd_type _Tag_, class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		static raze_always_inline auto __find(_Tag_, _Iterator_ __first, _Sentinel_ __last, 
			_Predicate_ __pred, _Projection_ __proj, sizetype __aligned_size, sizetype __tail_size) noexcept
		{
			auto* __ptr = std::to_address(__first);
			auto* __end = std::to_address(__last);

			raze_assume(__ptr != __nullptr);
			raze_assume(__end != __nullptr);
			raze_assume(__ptr + _Tag_::size() < __end);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				const auto __mask = __pred(__proj(raze::vx::load<_Tag_>(__ptr)));

				if (raze::vx::any_of(__mask))
					return __ptr + raze::vx::find_first_set(__mask);

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			for (; __ptr != __end; ++__ptr)
				if (__pred(__proj(*__ptr)))
					break;

			return __ptr;
		} 

		template <vx::simd_type _Tag_, class _Iterator_, class _Sentinel_, class _Predicate_,
			class _Projection_, sizetype _AlignedSize_, sizetype _TailSize_>
		static raze_always_inline auto __find(_Tag_, _Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred,
			_Projection_ __proj, std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>) noexcept 
		{
			auto* __ptr = std::to_address(__first);
			auto* __end = std::to_address(__last);

			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			raze_assume(__ptr <= __end);
			raze_assume(__ptr != nullptr);
			raze_assume(__end != nullptr);

			auto __left = __iterations_aligned;

			do {
				const auto __mask = __pred(__proj(raze::vx::load<_Tag_>(__ptr)));

				if (raze::vx::any_of(__mask))
					return __ptr + raze::vx::find_first_set(__mask);

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);
			

			if constexpr (_TailSize_ != 0) {
				do {
					if (__pred(__proj(*__ptr))) break;
					++__ptr;
				} while (__ptr != __end);
			}

			return __ptr;
		} 

		template <vx::simd_type _Tag_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, sizetype __aligned_size, sizetype __tail_size) noexcept {
			__seek_possibly_wrapped_iterator(_iterator, __find(_Tag_{}, _iterator, _sentinel, _predicate, _proj, __aligned_size, __tail_size));
		}

		template <vx::simd_type _Tag_, sizetype _AlignedSize_, sizetype _TailSize_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, std::integral_constant<sizetype, _AlignedSize_> __aligned_size,
			std::integral_constant<sizetype, _TailSize_> __tail_size) noexcept
		{
			__seek_possibly_wrapped_iterator(_iterator, __find(_Tag_{}, _iterator, _sentinel, _predicate, _proj,
				std::integral_constant<sizetype, _AlignedSize_>{}, std::integral_constant<sizetype, _TailSize_>{}));
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
	{
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj)));

		return __first;
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj), 
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(
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
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(_Iterator_ __first, 
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

constexpr inline auto find_if = raze::options::function_with_traits<_Find_if>;

__RAZE_ALGORITHM_NAMESPACE_END
