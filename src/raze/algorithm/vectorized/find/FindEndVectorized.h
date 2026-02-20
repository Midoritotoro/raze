#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_>
raze_always_inline const _Type_* __find_end_scalar(
	const void* __main_first,
	sizetype    __main_length,
	const void* __sub_first,
	sizetype    __sub_length) noexcept
{
	const auto* __main_current = static_cast<const _Type_*>(__main_first);
	const auto* __sub_current = static_cast<const _Type_*>(__sub_first);

	const auto __main_current_last = __main_current + __main_length;
	const auto __sub_current_last = __sub_current + __sub_length;

	for (auto __candidate = __main_current_last;; --__candidate) {
		auto __next1 = __candidate;
		auto __next2 = __sub_current_last;

		for (;;) {
			if (__sub_current == __next2) {
				__main_current = __next1;
				return __main_current;
			}

			if (__main_current == __next1)
				return __main_current_last;

			--__next1;
			--__next2;

			if (*__next1 != *__next2)
				break;
		}
	}
}

template <class _Simd_>
struct __find_end_vectorized_internal {
	using _ValueType = typename _Simd_::value_type;

	raze_always_inline raze_nodiscard raze_static_operator const _ValueType* operator()(
		sizetype	__aligned_size,
		sizetype	__tail_size,
		const void* __main_first,
		sizetype	__main_length,
		const void* __sub_first,
		sizetype	__sub_length) raze_const_operator noexcept
	{
		const auto __broadcasted_first_sub = _Simd_(static_cast<const _ValueType*>(__sub_first)[0]);
		const auto __broadcasted_last_sub = _Simd_(static_cast<const _ValueType*>(__sub_first)[__sub_length - 1]);

		const auto __sub_bytes = sizeof(_ValueType) * __sub_length;
		auto __main_last = __bytes_pointer_offset(__main_first, __aligned_size + __tail_size);

		const auto __stop_at = __bytes_pointer_offset(__main_last, -__aligned_size);
		const auto __cached_last = __main_last;

		while (__main_last != __stop_at) {
			__rewind_bytes(__main_last, sizeof(_Simd_));

			const auto __sequence_start = __bytes_pointer_offset(__main_last, -(__sub_bytes - sizeof(_ValueType)));
			const auto __sequence_end	= __main_last;

			const auto __start_loaded	= datapar::load<_Simd_>(__sequence_start);
			const auto __end_loaded		= datapar::load<_Simd_>(__sequence_end);

			const auto __equal_first = (__broadcasted_first_sub == __start_loaded) | datapar::as_mask;
			const auto __equal_last = (__broadcasted_last_sub == __end_loaded) | datapar::as_mask;

			auto __combined = (__equal_first & __equal_last);

			while (__combined.any_of()) {
				const auto __leading_zeros = __combined.count_leading_zero_bits();

				const auto __match_bytes = (__leading_zeros * sizeof(_ValueType) - sizeof(_ValueType));
				const auto __main_match = __bytes_pointer_offset(AAAa, __match_bytes);

				//const auto __l1 = datapar::load<_Simd_>(__main_match);
				//const auto __l2 = datapar::load<_Simd_>(__sub_first);

				if (memcmp(__main_match, __sub_first, __match_bytes) == 0)
					return static_cast<const _ValueType*>(__main_match);

				__combined.clear_left_most_set_bit();
			}
		}

		if (__tail_size == 0 || __sub_bytes > __tail_size)
			return static_cast<const _ValueType*>(__cached_last);

		/*if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
			__rewind_bytes(__main_last, __tail_size);

			const auto __tail_first_mask = datapar::make_tail_mask<_Simd_>(__tail_size);
			const auto __tail_last_mask = datapar::make_tail_mask<_Simd_>(__tail_size - __sub_bytes + sizeof(_ValueType));

			const auto __last_loaded = datapar::maskz_load<_Simd_>(__main_last, __tail_last_mask);
			const auto __first_loaded = datapar::maskz_load<_Simd_>(__bytes_pointer_offset(__main_last, __sub_bytes + sizeof(_ValueType)), __tail_first_mask);

			const auto __equal_first = (__broadcasted_first_sub == __first_loaded) | datapar::as_mask;
			const auto __equal_last = (__broadcasted_last_sub == __last_loaded) | datapar::as_mask;

			auto __combined = (__equal_first & __equal_last);

			while (__combined.any_of()) {
				const auto __leading_zeros = __combined.count_leading_zero_bits();
				const auto __main_match = __bytes_pointer_offset(__main_last, -(__leading_zeros * sizeof(_ValueType) + sizeof(_ValueType)));

				if (memcmp(__main_match, __bytes_pointer_offset(__sub_first, sizeof(_ValueType)), __sub_bytes - (2 * sizeof(_ValueType))) == 0)
					return static_cast<const _ValueType*>(__main_match) - 1;

				__combined.clear_left_most_set_bit();
			}

			return static_cast<const _ValueType*>(__cached_last);
		}
		else */{
			return __find_end_scalar<_ValueType>(__main_first, (__tail_size / sizeof(_ValueType)), __sub_first, __sub_length);
		}
	}
};

template <class _Type_>
raze_always_inline raze_nodiscard const _Type_* __find_end_vectorized(
	const void* __main_first,
	sizetype	__main_length,
	const void* __sub_first,
	sizetype	__sub_length) noexcept
{
	return datapar::__simd_sized_dispatcher<__find_end_vectorized_internal>::__apply<_Type_>(
		__main_length * sizeof(_Type_), &__find_end_scalar<_Type_>, __main_first, __main_length, __sub_first, __sub_length);
}

__RAZE_ALGORITHM_NAMESPACE_END
