#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/algorithm/SimdTransformable.h>



__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
	class _Type_,
	class _UnaryFunction_>
__raze_simd_algorithm_inline _Type_* __transform_scalar_unary(
	const void*			__first,
	const void*			__last,
	void*				__destination,
	_UnaryFunction_		__function) noexcept
{
	auto __first_current		= static_cast<const _Type_*>(__first);
	auto __destination_current	= static_cast<_Type_*>(__destination);

	for (; __first_current != __last; ++__first_current, ++__destination_current)
		*__destination_current = __function(*__first_current);

	return __destination_current;
}

template <
	class _Type_,
	class _BinaryFunction_>
__raze_simd_algorithm_inline _Type_* __transform_scalar_binary(
	const void*			__first1,
	const void*			__last1,
	const void*			__first2,
	void*				__destination,
	_BinaryFunction_	__function) noexcept
{
	auto __first1_current = static_cast<const _Type_*>(__first1);
	auto __first2_current = static_cast<const _Type_*>(__first2);

	auto __destination_current = static_cast<_Type_*>(__destination);

	for (; __first1_current != __last1; ++__first1_current, ++__first2_current, ++__destination_current)
		*__destination_current = __function(*__first1_current, *__first2_current);

	return __destination_current;
}

template <class _Simd_>
struct __transform_vectorized_internal {
	using _ValueType = typename _Simd_::value_type;

	template <class _UnaryFunction_>
	__raze_simd_algorithm_inline raze_static_operator _ValueType* operator()(
		sizetype			__aligned_size,
		sizetype			__tail_size,
		const void*			__first,
		const void*			__last,
		void*				__destination,
		_UnaryFunction_		__function) raze_const_operator noexcept
	{
		const auto __guard = datapar::make_guard<_Simd_>();
		const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

		if constexpr (is_simd_unary_transformable_v<_UnaryFunction_, _Simd_>) {
			do {
				datapar::store(__destination, __function(datapar::load<_Simd_>(__first)));

				__advance_bytes(__first, sizeof(_Simd_));
				__advance_bytes(__destination, sizeof(_Simd_));
			} while (__first != __stop_at);

			if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
				const auto __tail_mask = datapar::first_n<_Simd_>(__tail_size / sizeof(_ValueType));
				datapar::mask_store(__destination, __function(datapar::maskz_load<_Simd_>(__first, __tail_mask)), __tail_mask);

				__advance_bytes(__destination, __tail_size);
				return static_cast<_ValueType*>(__destination);
			}
			else {
				return __transform_scalar_unary<_ValueType>(__first, __last, __destination, __function);
			}
		}
		else {
			return __transform_scalar_unary<_ValueType>(__first, __last, __destination, __function);
		}
	}

	template <class _BinaryFunction_>
	__raze_simd_algorithm_inline raze_static_operator _ValueType* operator()(
		sizetype			__aligned_size,
		sizetype			__tail_size,
		const void*			__first1,
		const void*			__last1,
		const void*			__first2,
		void*				__destination,
		_BinaryFunction_	__function) raze_const_operator noexcept
	{
		const auto __guard = datapar::make_guard<_Simd_>();
		const auto __stop_at = __bytes_pointer_offset(__first1, __aligned_size);

		if constexpr (is_simd_binary_transformable_v<_BinaryFunction_, _Simd_>) {
			do {
				datapar::store(__destination, __function(
					datapar::load<_Simd_>(__first1), datapar::load<_Simd_>(__first2)));

				__advance_bytes(__first1, sizeof(_Simd_));
				__advance_bytes(__first2, sizeof(_Simd_));
				__advance_bytes(__destination, sizeof(_Simd_));
			} while (__first1 != __stop_at);

			/*if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
				const auto __tail_mask = datapar::first_n<_Simd_>(__tail_size / sizeof(_ValueType));
				datapar::mask_store(__destination, __function(
					datapar::maskz_load<_Simd_>(__first1, __tail_mask),
					datapar::maskz_load<_Simd_>(__first1, __tail_mask)), __tail_mask);

				__advance_bytes(__destination, __tail_size);
				return static_cast<_ValueType*>(__destination);
			}
			else*/ {
				return __transform_scalar_binary<_ValueType>(__first1, __last1, __first2, __destination, __function);
			}
		}
		else {
			return __transform_scalar_binary<_ValueType>(__first1, __last1, __first2, __destination, __function);
		}
	}
};

template <
	class _Type_, 
	class _UnaryFunction_>
__raze_simd_algorithm_inline _Type_* __transform_vectorized(
	const void*			__first,
	const void*			__last,
	void*				__destination,
	_UnaryFunction_		__function) noexcept
{
	return datapar::__simd_sized_dispatcher<__transform_vectorized_internal, _Type_>()(
		__byte_length(__first, __last), &__transform_scalar_unary<_Type_, _UnaryFunction_>,
			__first, __last, __destination, __function);
}

template <
	class _Type_, 
	class _BinaryFunction_>
__raze_simd_algorithm_inline _Type_* __transform_vectorized(
	const void*			__first1,
	const void*			__last1,
	const void*			__first2,
	void*				__destination,
	_BinaryFunction_	__function) noexcept
{
	return datapar::__simd_sized_dispatcher<__transform_vectorized_internal, _Type_>()(
		__byte_length(__first1, __last1), &__transform_scalar_binary<_Type_, _BinaryFunction_>,
			__first1, __last1, __first2, __destination, __function);
}

__RAZE_ALGORITHM_NAMESPACE_END
