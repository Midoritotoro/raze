#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_> 
raze_always_inline const _Type_* __search_scalar(
	const void* __main_first,
	sizetype	__main_length,
	const void* __sub_first,
	sizetype	__sub_length) noexcept
{
	const auto* __main_current	= static_cast<const _Type_*>(__main_first);
	const auto* __sub_current	= static_cast<const _Type_*>(__sub_first);

	if (__main_length == __sub_length)
		return (memcmp(__main_current, __sub_current, __main_length) == 0) ? __main_current : nullptr;

	const auto& __first = __sub_current[0];
	const auto __max_position = sizetype(__main_length) - sizetype(__sub_length) + 1;

	for (sizetype i = 0; i < __max_position; i++) {
		if ((__main_current[i] == __first) == false) {
			i++;

			while (i < __max_position && __main_current[i] != __first)
				i++;

			if (i == __max_position)
				break;
		}

		sizetype j = 1;

		for (; j < __sub_length; ++j)
			if (__main_current[i + j] != __sub_current[j])
				break;

		if (j == __sub_length)
			return (__main_current + i);
	}

	return nullptr;
}


template <class _Simd_>
struct __search_vectorized_internal {
	using _ValueType = typename _Simd_::value_type;

	raze_nodiscard raze_always_inline raze_static_operator const _ValueType* operator()(
		const void* __main_first,
		sizetype	__main_length,
		const void* __sub_first,
		sizetype	__sub_length) raze_const_operator noexcept
	{
		const auto __broadcasted_first_sub = _Simd_(static_cast<const _ValueType*>(__sub_first)[0]);
		const auto __broadcasted_last_sub = _Simd_(static_cast<const _ValueType*>(__sub_first)[__sub_length - 1]);
		
		const auto __main_bytes = sizeof(_ValueType) * __main_length;
		const auto __sub_bytes = sizeof(_ValueType) * __sub_length;

		const auto __main_aligned_size = __main_bytes & (~(sizeof(_Simd_) - 1));
		const auto __stop_at = __bytes_pointer_offset(__main_first, __main_aligned_size);

		while (__main_first != __stop_at) {
			const auto __loaded_first = datapar::load<_Simd_>(__main_first);
			const auto __loaded_last = datapar::load<_Simd_>(__bytes_pointer_offset(__main_first, __sub_bytes - sizeof(_ValueType)));

			const auto __equal_first = (__broadcasted_first_sub == __loaded_first) | datapar::as_mask;
			const auto __equal_last = (__broadcasted_last_sub == __loaded_last) | datapar::as_mask;

			auto __combined = __equal_first & __equal_last;
			
			while (__combined.any_of()) {
				const auto __trailing_zeros = __combined.count_trailing_zero_bits();
				const auto __main_match = __bytes_pointer_offset(__main_first, __trailing_zeros * sizeof(_ValueType) + sizeof(_ValueType));
				
				if (memcmp(__main_match, __bytes_pointer_offset(__sub_first, sizeof(_ValueType)), __sub_bytes - (2 * sizeof(_ValueType))) == 0)
					return static_cast<const _ValueType*>(__main_match) - 1;

				__combined.clear_left_most_set_bit();
			}

			__advance_bytes(__main_first, sizeof(_Simd_));
		}

		const auto __tail_size = __main_bytes & (sizeof(_Simd_) - sizeof(_ValueType));

		if (__tail_size == 0 || __sub_bytes > __tail_size)
			return static_cast<const _ValueType*>(__main_first) + __main_length;

		if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
			const auto __tail_first_mask = datapar::make_tail_mask<_Simd_>(__tail_size);
			const auto __tail_last_mask = datapar::make_tail_mask<_Simd_>(__tail_size - __sub_bytes + sizeof(_ValueType));

			const auto __loaded_first	= datapar::maskz_load<_Simd_>(__main_first, __tail_first_mask);
			const auto __loaded_last	= datapar::maskz_load<_Simd_>(__bytes_pointer_offset(__main_first, __sub_bytes - sizeof(_ValueType)), __tail_last_mask);

			const auto __equal_first = (__broadcasted_first_sub == __loaded_first) | datapar::as_mask;
			const auto __equal_last = (__broadcasted_last_sub == __loaded_last) | datapar::as_mask;

			auto __combined = __equal_first & __equal_last;

			while (__combined.any_of()) {
				const auto __trailing_zeros = __combined.count_trailing_zero_bits();

				const auto __main_match = __bytes_pointer_offset(__main_first, __trailing_zeros * sizeof(_ValueType) + sizeof(_ValueType));

				if (memcmp(__main_match, __bytes_pointer_offset(__sub_first, sizeof(_ValueType)), __sub_bytes - (2 * sizeof(_ValueType))) == 0)
					return static_cast<const _ValueType*>(__main_match) - 1;

				__combined.clear_left_most_set_bit();
			}
		}
		else {
			return __search_scalar<_ValueType>(__main_first, __tail_size / sizeof(_ValueType), __sub_first, __sub_length);
		}
	}
};

template <class _Type_>
raze_always_inline const _Type_* __search_vectorized(
	const void* __main_first,
	sizetype	__main_length,
	const void* __sub_first,
	sizetype	__sub_length) noexcept
{
	return datapar::__simd_dispatcher<__search_vectorized_internal>::__apply<_Type_>(
		&__search_scalar<_Type_>, __main_first, __main_length, __sub_first, __sub_length);
}

__RAZE_ALGORITHM_NAMESPACE_END
