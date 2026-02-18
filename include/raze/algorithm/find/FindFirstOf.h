#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/type_traits/SimdAlgorithmSafety.h>

#include <src/raze/algorithm/vectorized/find/FindVectorized.h>
#include <src/raze/algorithm/vectorized/find/SearchVectorized.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <raze/concurrency/Execution.h>

#include <src/raze/datapar/IsComparable.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _FirstForwardIterator_ find_first_of(
	_FirstForwardIterator_			__first1, 
	const _FirstForwardIterator_	__last1, 
	const _SecondForwardIterator_	__first2,
	const _SecondForwardIterator_	__last2, 
	_Predicate_						__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	__verify_range(__first1, __last1);
	__verify_range(__first2, __last2);

	using _FirstForwardUnwrappedIterator_	= __unwrapped_iterator_type<_FirstForwardIterator_>;
	using _SecondForwardUnwrappedIterator_	= __unwrapped_iterator_type<_SecondForwardIterator_>;

	auto __first1_unwrapped			= __unwrap_iterator(__first1);
	auto __last1_unwrapped			= __unwrap_iterator(__last1);

	auto __first2_unwrapped			= __unwrap_iterator(__first2);
	const auto __last2_unwrapped	= __unwrap_iterator(__last2);

	if constexpr (
		type_traits::is_iterator_random_ranges_v<_SecondForwardUnwrappedIterator_> &&
		std::is_same_v<_Predicate_, type_traits::equal_to<>>) 
	{
		const auto __length = __iterators_difference(__first2_unwrapped, __last2_unwrapped);

		if (__length == 1) {
			const auto __value = *__first2_unwrapped;
			using _ValueType = type_traits::iterator_value_type<_SecondForwardUnwrappedIterator_>;

			if (datapar::__is_comparable<_FirstForwardUnwrappedIterator_>(__value) == false)
				return __last1;

			const auto __first1_address = std::to_address(__first1_unwrapped);
			const void* __position = nullptr;

			if constexpr (type_traits::is_vectorized_find_algorithm_safe_v<_FirstForwardUnwrappedIterator_, _ValueType>)
				__position = __find_vectorized(__first1_address, std::to_address(__last1_unwrapped), __value);
			else
				__position = __find_scalar(__first1_address, std::to_address(__last1_unwrapped), __value);

			if constexpr (std::is_pointer_v<_FirstForwardIterator_>)
				return reinterpret_cast<_ValueType*>(__position);
			else
				return __first1 + (reinterpret_cast<const _ValueType*>(__position) - __first1_address);
		}
	}

//	if constexpr (type_traits::is_vectorized_search_algorithm_safe_v<
//		_FirstForwardUnwrappedIterator_, _SecondForwardUnwrappedIterator_, _Predicate_>) 
//	{
//#if raze_has_cxx20
//        if (type_traits::is_constant_evaluated() == false) 
//#endif // raze_has_cxx20
//		{
//			using _ValueType_ = type_traits::IteratorValueType<_SecondForwardUnwrappedIterator_>;
//
//            const auto first1Address	= std::to_address(first1Unwrapped);
//           
//			if constexpr (std::is_pointer_v<_FirstForwardIterator_>)
//				return position;
//			else
//				return first1 + static_cast<type_traits::IteratorDifferenceType<_FirstForwardIterator_>>(position - first1Address);
//        }
//    }

	for (; __first1_unwrapped != __last1_unwrapped; ++__first1_unwrapped) {
        for (auto __mid2_unwrapped = __first2_unwrapped; __mid2_unwrapped != __last2_unwrapped; ++__mid2_unwrapped) {
            if (__predicate(*__first1_unwrapped, *__mid2_unwrapped)) {
				__seek_possibly_wrapped_iterator(__first1, __first1_unwrapped);
                return __first1;
            }
        }
    }

	__seek_possibly_wrapped_iterator(__first1, __first1_unwrapped);
    return __first1;
}


template <
	class _FirstForwardIterator_,
	class _SecondForwardIterator_>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _FirstForwardIterator_ find_first_of(
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_	__first2,
	_SecondForwardIterator_	__last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::find_first_of(__first1, __last1, __first2, __last2, type_traits::equal_to<>{});
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	class _Predicate_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _FirstForwardIterator_ find_first_of(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1, 
	_FirstForwardIterator_	__last1, 
	_SecondForwardIterator_	__first2,
	_SecondForwardIterator_	__last2, 
	_Predicate_				__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::find_first_of(__first1, __last1, __first2, __last2, type_traits::__pass_function(__predicate));
}

template <
	class _ExecutionPolicy_,
	class _FirstForwardIterator_,
	class _SecondForwardIterator_,
	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
raze_nodiscard raze_constexpr_cxx20 raze_always_inline _FirstForwardIterator_ find_first_of(
	_ExecutionPolicy_&&,
	_FirstForwardIterator_	__first1,
	_FirstForwardIterator_	__last1,
	_SecondForwardIterator_	__first2,
	_SecondForwardIterator_	__last2) noexcept(
		type_traits::is_nothrow_invocable_v<
			type_traits::equal_to<>,
			type_traits::iterator_value_type<_FirstForwardIterator_>,
			type_traits::iterator_value_type<_SecondForwardIterator_>>)
{
	return raze::algorithm::find_first_of(__first1, __last1, __first2, __last2);
}

__RAZE_ALGORITHM_NAMESPACE_END
