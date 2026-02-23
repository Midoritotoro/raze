#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_>
raze_nodiscard raze_declare_const_function __raze_simd_algorithm_inline const _Type_* __find_end_scalar(
	const void* __main_first,
	sizetype    __main_length,
	const void* __sub_first,
	sizetype    __sub_length) noexcept
{
	const auto* main = static_cast<const _Type_*>(__main_first);
	const auto* sub = static_cast<const _Type_*>(__sub_first);

	const auto* main_end = main + __main_length;
	const auto* sub_end = sub + __sub_length;

	for (const _Type_* cand = main_end - __sub_length; cand >= main; --cand) {
		const _Type_* p1 = cand;
		const _Type_* p2 = sub;

		while (p2 != sub_end && *p1 == *p2) {
			++p1;
			++p2;
		}

		if (p2 == sub_end)
			return cand;
	}

	return main_end;
}


template <class _Simd_>
struct __find_end_vectorized_internal {
	using _ValueType = typename _Simd_::value_type;

	raze_nodiscard raze_declare_const_function __raze_simd_algorithm_inline raze_static_operator const _ValueType* operator()(
		sizetype	__aligned_size,
		sizetype	__tail_size,
		const void* __main_first,
		sizetype	__main_length,
		const void* __sub_first,
		sizetype	__sub_length) raze_const_operator noexcept
	{
		const auto __guard = datapar::make_guard<_Simd_>();

		const auto __broadcasted_first_sub = _Simd_(static_cast<const _ValueType*>(__sub_first)[0]);
		const auto __broadcasted_last_sub = _Simd_(static_cast<const _ValueType*>(__sub_first)[__sub_length - 1]);

		const auto __sub_bytes = sizeof(_ValueType) * __sub_length;
		const auto __main_bytes = __aligned_size + __tail_size;
		const auto __last_offset = int64(__sub_bytes);

		auto __main_last = __bytes_pointer_offset(__main_first, __aligned_size + __tail_size);
		const auto __cached_last = __main_last;

		const auto __stop_at = __bytes_pointer_offset(__main_last, -__aligned_size);

		auto __processed_bytes = sizetype(0);

		while (__processed_bytes < __aligned_size && (__main_bytes - __processed_bytes) >= (__last_offset + sizeof(_Simd_))) {
			__rewind_bytes(__main_last, sizeof(_Simd_));
			__processed_bytes += sizeof(_Simd_);

			const auto __loaded_last = datapar::load<_Simd_>(__main_last);
			const auto __equal_last = (__loaded_last == __broadcasted_last_sub);

			if (datapar::none_of(__equal_last))
				continue;

			const auto __sequence_start = __bytes_pointer_offset(__main_last, -(__last_offset - sizeof(_ValueType)));

			const auto __loaded_first = datapar::load<_Simd_>(__sequence_start);
			const auto __equal_first = (__loaded_first == __broadcasted_first_sub);

			const auto __combined = __equal_first & __equal_last;

			if (datapar::any_of(__combined)) {
				auto __mask = datapar::to_mask(__combined);

				do {
					const auto __offset_count = (__mask.bit_width() - __mask.count_leading_zero_bits());
					const auto __offset_bytes = __offset_count * sizeof(_ValueType);

					const auto __main_match = __bytes_pointer_offset(__sequence_start, __offset_bytes);

					if (memcmp(__main_match, __bytes_pointer_offset(__sub_first, sizeof(_ValueType)), __sub_bytes - 2 * sizeof(_ValueType)) == 0)
						return static_cast<const _ValueType*>(__main_match) - 1;

					__mask.clear_left_most_set_bit();
				} while (datapar::any_of(__mask));
			}
		}

		const auto __remaining_bytes = __main_bytes - __processed_bytes; 
		
		if (__remaining_bytes < __sub_bytes)
			return static_cast<const _ValueType*>(__cached_last);
		
		const auto __remaining_length = __remaining_bytes / sizeof(_ValueType); 

		const auto __main_begin = static_cast<const _ValueType*>(__main_first); 
		const auto __main_global_end = static_cast<const _ValueType*>(__cached_last); 

		const auto __scalar_result = __find_end_scalar<_ValueType>(__main_first, __remaining_length, __sub_first, __sub_length); 
		
		if (__scalar_result == __main_begin + __remaining_length) 
			return __main_global_end; 
		
		return __scalar_result;
	}
};

template <class _Type_>
raze_nodiscard raze_declare_const_function __raze_simd_algorithm_inline const _Type_* __find_end_vectorized(
	const void* __main_first,
	sizetype	__main_length,
	const void* __sub_first,
	sizetype	__sub_length) noexcept
{
	return datapar::__simd_sized_dispatcher<__find_end_vectorized_internal>::__apply<_Type_>(
		__main_length * sizeof(_Type_), &__find_end_scalar<_Type_>, __main_first, __main_length, __sub_first, __sub_length);
}

__RAZE_ALGORITHM_NAMESPACE_END
