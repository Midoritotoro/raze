#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__raze_simd_algorithm_inline void __swap_ranges_scalar(
	void*		__first,
	void*		__second,
	sizetype	__count) noexcept
{
	auto* __first_current = static_cast<_Type_*>(__first);
	auto* __second_current = static_cast<_Type_*>(__second);

	for (auto __current = sizetype(0); __current < __count; ++__current) {
		_Type_ __temp = *__first_current;

		*__first_current++ = *__second_current;
		*__second_current++ = __temp;
	}
}

template <class _Simd_>
struct __swap_ranges_vectorized_internal {
	using _ValueType = typename _Simd_::value_type;

	raze_static_operator __raze_simd_algorithm_inline void operator()(
		sizetype								__aligned_size,
		sizetype								__tail_size,
		void*									__first,
		void*									__second,
		raze_maybe_unused_attribute sizetype	__count) raze_const_operator noexcept
	{	
		const auto __guard = datapar::make_guard<_Simd_>();
		const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

		do {
			const auto __loaded_first	= datapar::load<_Simd_>(__first);
			const auto __loaded_second	= datapar::load<_Simd_>(__second);

			datapar::store(__second, __loaded_first);
			datapar::store(__first, __loaded_second);

			__advance_bytes(__first, sizeof(_Simd_));
			__advance_bytes(__second, sizeof(_Simd_));
		} while (__first != __stop_at);

		return __swap_ranges_scalar<_ValueType>(__first, __second, __tail_size / sizeof(_ValueType));
	}
};


template <typename _Type_>
__raze_simd_algorithm_inline void __swap_ranges_vectorized(
	void*		__first,
	void*		__second,
	sizetype	__count) noexcept
{
	using _IntegerType = typename IntegerForSizeof<_Type_>::Unsigned;

	datapar::__simd_sized_dispatcher<__swap_ranges_vectorized_internal, _IntegerType>()(
		__count * sizeof(_IntegerType), &__swap_ranges_scalar<_IntegerType>, __first, __second, __count);
}

__RAZE_ALGORITHM_NAMESPACE_END
