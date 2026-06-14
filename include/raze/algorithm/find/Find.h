#pragma once 

#include <src/raze/algorithm/unchecked/find/FindUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfNotUnchecked.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_ = std::iter_value_t<_Iterator_>, class _Projection_ = std::identity>
	constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const std::type_identity_t<_Value_>& __value, _Projection_ __proj = {}) const noexcept
	{
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)), __value, __proj));
		return __first;
	}

	template <std::ranges::input_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(
		_Range_&& __range, const std::type_identity_t<_Value_>& __value, _Projection_ __proj = {}) const noexcept
	{
		return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			__value, type_traits::__pass_function(__proj));
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	raze_nodiscard static constexpr _Iterator_ __find_unchecked(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __value, _Projection_ __proj)
	{
		__verify_range(__first, __last);

		if constexpr (std::is_same_v<_Projection_, std::identity> && std::sized_sentinel_for<_Sentinel_, _Iterator_>
			&& type_traits::__is_vectorized_find_algorithm_safe_v<_Iterator_, _Value_>) 
		{
			if not consteval {
				const auto __first_address = std::to_address(__first);
				const auto __position = __find_vectorized(__first_address, std::to_address(__last), __value);

				if constexpr (std::is_pointer_v<_Iterator_>) return __position;
				else return __first + (__position - __first_address);
			}
		}

		for (; __first != __last; ++__first)
			if (*__first == __value)
				break;

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
