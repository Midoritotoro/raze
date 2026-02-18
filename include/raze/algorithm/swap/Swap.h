#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SwapSafety.h>

#include <src/raze/algorithm/vectorized/swap/SwapRangesVectorized.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
constexpr void swap(
	_Type_& __first,
	_Type_& __second) noexcept(
		std::is_nothrow_move_constructible<_Type_>::value &&
		std::is_nothrow_move_assignable<_Type_>::value
	)
{
	auto __temp = __first;

	__first		= std::move(__second);
	__second	= std::move(__temp);
}

template <
	typename _FirstForwardIterator_,
	typename _SecondForwardIterator_>
constexpr void iter_swap(
	_FirstForwardIterator_	__first,
	_SecondForwardIterator_ __second) noexcept(noexcept(swap(*__first, *__second)))
{
	swap(*__first, *__second);
}

#if defined(raze_cpp_clang) || defined(__EDG__)
	void swap() = delete;
#else
	void swap();
#endif

template <
	class,
	class = void>
struct __has_adl_swap:
	std::false_type
{};

template <class _Type_>
struct __has_adl_swap<_Type_, std::void_t<decltype(swap(
	std::declval<_Type_&>(),
	std::declval<_Type_&>()))>
>:
	std::true_type
{};

template <class _Type_>
constexpr bool is_trivially_swappable_v = std::conjunction_v<
	std::is_trivially_destructible<_Type_>,
	std::is_trivially_move_constructible<_Type_>,
	std::is_trivially_move_assignable<_Type_>,
	std::negation<std::_Has_ADL_swap_detail::_Has_ADL_swap<_Type_>
>>;

#ifdef __cpp_lib_byte
template <>
inline constexpr bool is_trivially_swappable_v<std::byte> = true;
#endif // defined(__cpp_lib_byte)


template <
	class		_Type_,
	sizetype	_Length_> 
constexpr void swap(
	_Type_ (&__first)[_Length_],
	_Type_ (&__second)[_Length_]) noexcept(noexcept(swap(*__first, *__second)))
{
	if (&__first == &__second)
		return;

	if constexpr (is_trivially_swappable_v<_Type_>) {
#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			return __swap_ranges_vectorized<_Type_>(__first, __second, _Length_);
		}
	}
	else {
		for (auto __current = sizetype(0); __current < _Length_; ++__current)
			swap(__first[__current], __second[__current]);
	}
}

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_>
constexpr _FirstForwardIterator_ swap_ranges(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2) noexcept
{
	using _ValueType_ = type_traits::iterator_value_type<_FirstForwardIterator_>;
	__verify_range(__first1, __last1);

	const auto __first1_unwrapped	= __unwrap_iterator(__first1);
	const auto __last1_unwrapped	= __unwrap_iterator(__last1);

	const auto __first2_unwrapped	= __unwrap_iterator(__first2);

	if constexpr (is_trivially_swappable_v<_ValueType_>) {
		const auto __difference = __iterators_difference(__first1_unwrapped, __last1_unwrapped);

#if raze_has_cxx20
		if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
		{
			__swap_ranges_vectorized<_ValueType_>(std::to_address(__first1_unwrapped),
				std::to_address(__first2_unwrapped), __difference);
		}

		__seek_possibly_wrapped_iterator(__first2, __first2 + __difference);
	}
	else {
		for (; __first1_unwrapped != __last1_unwrapped; ++__first1_unwrapped, ++__first2_unwrapped)
			swap(*__first1_unwrapped, *__first2_unwrapped);

		__seek_possibly_wrapped_iterator(__first2, __first2_unwrapped);
	}

	return __first2;
}

template <
	class _ExecutionPolicy_,	
	class _FirstForwardIterator_,
	class _SecondForwardIterator_>
constexpr _FirstForwardIterator_ swap_ranges(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_ __first2) noexcept
{
	return raze::algorithm::swap_ranges(__first1, __last1, __first2);
}

__RAZE_ALGORITHM_NAMESPACE_END
