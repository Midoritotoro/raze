#pragma once 

#include <src/raze/algorithm/unchecked/find/FindUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfNotUnchecked.h>

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Value_ _value;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Value_ __v, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _value(__v), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			else if (_proj(*_iterator) == _value) return true;
			else {
				++_iterator;
				return false;
			}
		}

		template <vx::simd_type _Tag_, class _Iterator_, class _Sentinel_, class _Value_>
		static raze_always_inline auto __find(_Tag_, _Iterator_ __first,
			_Sentinel_ __last, const _Value_& __v, sizetype __aligned_size, sizetype __tail_size) noexcept 
		{
			auto* __ptr = std::to_address(__first);
			auto* __end = std::to_address(__last);

			raze_assume(__ptr != __nullptr);
			raze_assume(__end != __nullptr);
			raze_assume(__ptr + _Tag_::size() < __end);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				const auto __mask = __v == raze::vx::load<_Tag_>(__ptr);

				if (raze::vx::any_of(__mask))
					return __ptr + raze::vx::find_first_set(__mask);

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			for (; __ptr != __end; ++__ptr)
				if (*__ptr == __v)
					break;

			return __ptr;
		} 

		template <vx::simd_type _Tag_, class _Iterator_, class _Sentinel_, class _Value_,
			sizetype _AlignedSize_, sizetype _TailSize_>
		static raze_always_inline auto __find(_Tag_, _Iterator_ __first, _Sentinel_ __last, const _Value_& __v, 
			std::integral_constant<sizetype, _AlignedSize_>, std::integral_constant<sizetype, _TailSize_>) noexcept 
		{
			auto* __ptr = std::to_address(__first);
			auto* __end = std::to_address(__last);

			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			raze_assume(__ptr <= __end);
			raze_assume(__ptr != nullptr);

			if constexpr (__iterations_aligned == 2) {
				using _Tag_x2_ = vx::simd<typename _Tag_::value_type, vx::resize_abi_t<vx::abi_t<_Tag_>, _Tag_::size() * 2>>;

				raze_assume(__ptr + sizeof(_Tag_x2_) <= __end);

				const auto __mask = __v == raze::vx::load<_Tag_x2_>(__ptr);

				if (raze::vx::any_of(__mask))
					return __ptr + raze::vx::find_first_set(__mask);

				__advance_bytes(__ptr, sizeof(_Tag_x2_));
			}
			else {
				auto __left = __iterations_aligned;

				do {
					const auto __mask = __v == raze::vx::load<_Tag_>(__ptr);

					if (raze::vx::any_of(__mask))
						return __ptr + raze::vx::find_first_set(__mask);

					__advance_bytes(__ptr, sizeof(_Tag_));
				} while (--__left);
			}

			if constexpr (_TailSize_ != 0) {
				raze_assume(__end != nullptr);
				raze_assume(__end >= __ptr);

				do {
					if (*__ptr++ == __v) break;
				} while (__ptr != __end);
			}

			return __ptr;
		} 

		template <vx::simd_type _Tag_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, sizetype __aligned_size, sizetype __tail_size) noexcept {
			__seek_possibly_wrapped_iterator(_iterator, __find(_Tag_{}, _iterator, _sentinel, _value, __aligned_size, __tail_size));
		}

		template <vx::simd_type _Tag_, sizetype _AlignedSize_, sizetype _TailSize_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, std::integral_constant<sizetype, _AlignedSize_> __aligned_size,
			std::integral_constant<sizetype, _TailSize_> __tail_size) noexcept
		{
			__seek_possibly_wrapped_iterator(_iterator, __find(_Tag_{}, _iterator, _sentinel, _value, 
				std::integral_constant<sizetype, _AlignedSize_>{}, std::integral_constant<sizetype, _TailSize_>{}));
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_ = std::iter_value_t<_Iterator_>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)), __v, __proj));

		return __first;
	}

	/*template <std::ranges::input_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(
		_Range_&& __range, const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			__v, type_traits::__pass_function(__proj));
	}*/

	template <constexpr_sized_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(_Range_&& __range,
		const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			__v, type_traits::__pass_function(__proj), std::integral_constant<sizetype,
			__range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(
		_Iterator_ __first, _Sentinel_ __last, const _Value_& __v, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		auto __work = __impl(__first, __last, __v, __proj);

		if consteval {
			__seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work));
		}
		else {
			__seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl,
				_Value_, _Iterator_>(algorithm::distance(__first, __last) * sizeof(_Value_), __work));
		}

		return __first;
	}

	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, const _Value_& __v, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		auto __work = __impl(__first, __last, __v, __proj);

		if consteval {
			__seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work));
		}
		else {
			__seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl,
				_Value_, _Iterator_>(std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{}, __work));
		}

		return __first;
	}
};

constexpr inline auto find = raze::options::function_with_traits<_Find>;

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputIterator_ find_if_not(
	_InputIterator_	__first, 
	_InputIterator_	__last, 
	_Predicate_		__predicate) noexcept(
		std::is_nothrow_invocable_v<
			_Predicate_, std::iter_value_t<_InputIterator_>>)
{
	__verify_range(__first, __last);

	__seek_possibly_wrapped_iterator(__first, __find_if_not_unchecked(__unwrap_iterator(__first),
		__unwrap_iterator(__last), type_traits::__pass_function(__predicate)));

	return __first;
}

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputIterator_ find_if(
	_InputIterator_	__first,
	_InputIterator_	__last,
	_Predicate_		__predicate) noexcept(
		std::is_nothrow_invocable_v<
			_Predicate_, std::iter_value_t<_InputIterator_>>)
{
	__verify_range(__first, __last);

	__seek_possibly_wrapped_iterator(__first, __find_if_unchecked(__unwrap_iterator(__first),
		__unwrap_iterator(__last), type_traits::__pass_function(__predicate)));

	return __first;
}

//template <
//	class _ExecutionPolicy_,
//	class _Iterator_,
//	class _Type_ = std::iter_value_t<_Iterator_>,
//	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
//raze_nodiscard _Iterator_ find(
//	_ExecutionPolicy_&&,
//	_Iterator_											__first,
//	_Iterator_											__last,
//	const typename std::type_identity<_Type_>::type&	__value) noexcept;

__RAZE_ALGORITHM_NAMESPACE_END
