#pragma once 

#include <src/raze/algorithm/AlgorithmDebug.h>

#include <src/raze/algorithm/vectorized/find/SearchVectorized.h>
#include <src/raze/type_traits/CanMemcmpElements.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/type_traits/OperatorWrappers.h>

#include <src/raze/algorithm/vectorized/find/FindVectorized.h>
#include <src/raze/algorithm/vectorized/find/EqualVectorized.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _FirstUnwrappedForwardIterator_,
    class _SecondUnwrappedForwardIterator_,
    class _DifferenceType_,
    class _Predicate_>
__simd_nodiscard_inline_constexpr _FirstUnwrappedForwardIterator_ __search_unchecked_random(
    _FirstUnwrappedForwardIterator_     __first1_unwrapped,
    _FirstUnwrappedForwardIterator_     __last1_unwrapped,
    _SecondUnwrappedForwardIterator_    __first2_unwrapped,
    _SecondUnwrappedForwardIterator_    __last2_unwrapped,
    _DifferenceType_                    __first_range_length,
    _DifferenceType_                    __second_range_length,
    _Predicate_                         __predicate) noexcept(
        type_traits::is_nothrow_invocable_v<
            _Predicate_,
            type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>,
            type_traits::iterator_value_type<_SecondUnwrappedForwardIterator_>>)
{
    const auto __max_position = static_cast<_DifferenceType_>(__first_range_length - __second_range_length + 1);
    const auto& __first = *__first2_unwrapped;

    for (_DifferenceType_ i = 0; i < __max_position; ++i) {

        auto __position = __first1_unwrapped + i;

        if (!__predicate(*__position, __first)) {
            ++i;
            while (i < __max_position &&
                   !__predicate(*(__first1_unwrapped + i), __first))
                ++i;

            if (i == __max_position)
                break;

			__position = __first1_unwrapped + i;
        }

        auto __first_iterator = __position + 1;
        auto __second_iterator = __first2_unwrapped + 1;

        for (; __second_iterator != __last2_unwrapped; ++__first_iterator, ++__second_iterator)
            if (!__predicate(*__first_iterator, *__second_iterator))
                break;

        if (__second_iterator == __last2_unwrapped)
            return __position;
    }

    return __last1_unwrapped;
}

template <
	class _FirstUnwrappedForwardIterator_,
	class _SecondUnwrappedForwardIterator_,
	class _Predicate_> 
__simd_nodiscard_inline_constexpr _FirstUnwrappedForwardIterator_ __search_unchecked_forward(
	_FirstUnwrappedForwardIterator_		__first1_unwrapped,
	_FirstUnwrappedForwardIterator_		__last1_unwrapped,
	_SecondUnwrappedForwardIterator_	__first2_unwrapped,
	_SecondUnwrappedForwardIterator_	__last2_unwrapped,
	_Predicate_							__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedForwardIterator_>>)
{
	for (;; ++__first1_unwrapped) {
		auto __current_first = __first1_unwrapped;
		
		for (auto __current_second = __first2_unwrapped;;  (void) ++__current_first,  (void) ++__current_second) {
			if (__current_second == __last2_unwrapped)
				return __first1_unwrapped;

			if (__current_first == __last1_unwrapped)
				return __last1_unwrapped;

			if (__predicate(*__current_first, *__current_second) == false)
				break;
		}
	}

	return __last1_unwrapped;
}

template <
	class _FirstUnwrappedForwardIterator_,
	class _SecondUnwrappedForwardIterator_,
	class _Predicate_> 
__simd_nodiscard_inline_constexpr _FirstUnwrappedForwardIterator_ __search_unchecked(
	_FirstUnwrappedForwardIterator_		__first1_unwrapped,
	_FirstUnwrappedForwardIterator_		__last1_unwrapped,
	_SecondUnwrappedForwardIterator_	__first2_unwrapped,
	_SecondUnwrappedForwardIterator_	__last2_unwrapped,
	_Predicate_							__predicate) noexcept(
		type_traits::is_nothrow_invocable_v<
			_Predicate_,
			type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>,
			type_traits::iterator_value_type<_SecondUnwrappedForwardIterator_>>)
{
	using _ValueType = type_traits::iterator_value_type<_FirstUnwrappedForwardIterator_>;

	constexpr auto __is_random = type_traits::is_iterator_random_ranges_v<_FirstUnwrappedForwardIterator_> &&
		type_traits::is_iterator_random_ranges_v<_SecondUnwrappedForwardIterator_>;
		
	if constexpr (__is_random) {
		auto __first_range_length			= __iterators_difference(__first1_unwrapped, __last1_unwrapped);
		const auto __second_range_length	= __iterators_difference(__first2_unwrapped, __last2_unwrapped);

		if constexpr (type_traits::__is_vectorized_search_algorithm_safe_v<
			_FirstUnwrappedForwardIterator_, _SecondUnwrappedForwardIterator_, _Predicate_>)
		{
#if raze_has_cxx20
			if (type_traits::is_constant_evaluated() == false)
#endif // raze_has_cxx20
			{
				const auto __first1_address = std::to_address(__first1_unwrapped);

				if (__first_range_length < __second_range_length)
					return __last1_unwrapped;

				else if (raze_unlikely(__second_range_length == 0))
					return __first1_unwrapped;

				else if (__second_range_length == 1)
					return __find_vectorized(__first1_address, __first1_address + __first_range_length, *__first2_unwrapped);

				else if (__second_range_length == __first_range_length)
					return (__equal_vectorized<_ValueType>(__first1_address, std::to_address(__first2_unwrapped), __first_range_length) == false) ? __last1_unwrapped : __first1_unwrapped;

				const auto __position = __search_vectorized<_ValueType>(__first1_address, 
					__first_range_length, std::to_address(__first2_unwrapped), __second_range_length);

				return __first1_unwrapped + (reinterpret_cast<const _ValueType*>(__position) - __first1_address);
			}
		}
		
		return __search_unchecked_random(__first1_unwrapped, __last1_unwrapped,
			__first2_unwrapped, __last2_unwrapped, __first_range_length,
			__second_range_length, type_traits::__pass_function(__predicate));
	}
	else {
		return __search_unchecked_forward(__first1_unwrapped, __last1_unwrapped,
			__first2_unwrapped, __last2_unwrapped, type_traits::__pass_function(__predicate));
	}
}

__RAZE_ALGORITHM_NAMESPACE_END
