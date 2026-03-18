#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_>
raze_nodiscard __raze_simd_algorithm_inline const _Type_* __find_first_of_scalar(
	const void* __first1,
	const void* __last1,
	const void* __first2,
	const void* __last2) noexcept
{
	auto __first1_current = static_cast<const _Type_*>(__first1);
	auto __first2_current = static_cast<const _Type_*>(__first2);

	for (; __first1_current != __last1; ++__first1_current)
		for (auto __mid2_unwrapped = __first2_current; __mid2_unwrapped != __last2; ++__mid2_unwrapped)
			if (*__first1_current == *__mid2_unwrapped)
				return __first1_current;

	return static_cast<const _Type_*>(__last1);
}

template <class _Simd_>
struct __find_first_of_vectorized_internal {
	using _ValueType = typename _Simd_::value_type;

	raze_nodiscard raze_static_operator __raze_simd_algorithm_inline const _ValueType* operator()(
		sizetype	__aligned_size,
		sizetype	__tail_size,
		const void* __first1,
		const void* __last1,
		const void* __first2,
		const void* __last2) raze_const_operator noexcept
	{
		const auto __guard = datapar::make_guard<_Simd_>();

		const auto __stop_at = __bytes_pointer_offset(__first1, __aligned_size);

		do {

			__advance_bytes(__first1, sizeof(_Simd_));
		} while (__first1 != __stop_at);
	}
};

template <class _Type_> 
raze_nodiscard __raze_simd_algorithm_inline const _Type_* __find_first_of_vectorized(
	const void* __first1,
	const void* __last1,
	const void* __first2,
	const void* __last2) noexcept
{
	return datapar::__simd_sized_dispatcher<__find_first_of_vectorized_internal, _Type_>()(
		__byte_length(__first1, __last1), &__find_first_of_scalar<_Type_>, __first1, __last1, __first2, __last2);
}

__RAZE_ALGORITHM_NAMESPACE_END
