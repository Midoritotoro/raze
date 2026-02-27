#pragma once 

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_> 
__raze_simd_algorithm_inline const _Type_* __search_scalar(
	const void* __main_first,
	sizetype	__main_length,
	const void* __sub_first,
	sizetype	__sub_length) noexcept
{
	const auto* __main_current	= static_cast<const _Type_*>(__main_first);
	const auto* __sub_current	= static_cast<const _Type_*>(__sub_first);

	const auto __first = __sub_current[0];
	const auto __max_position = sizetype(__main_length) - sizetype(__sub_length) + 1;

	for (sizetype __i = 0; __i < __max_position; __i++) {
		if (__main_current[__i] != __first) {
            __i++;

			while (__i < __max_position && __main_current[__i] != __first)
                __i++;

			if (__i == __max_position)
				break;
		}

		auto __j = sizetype(1);

		for (; __j < __sub_length; ++__j)
			if (__main_current[__i + __j] != __sub_current[__j])
				break;

		if (__j == __sub_length)
			return (__main_current + __i);
	}

	return static_cast<const _Type_*>(__main_first) + __main_length;
}


template <class _Simd_>
struct __search_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_static_operator __raze_simd_algorithm_inline const _ValueType* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __main_first,
        sizetype    __main_length,
        const void* __sub_first,
        sizetype    __sub_length) raze_const_operator noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();

        auto __main_begin = static_cast<const _ValueType*>(__main_first);
        auto __sub_begin = static_cast<const _ValueType*>(__sub_first);

        const auto __broadcasted_first_sub = _Simd_(__sub_begin[0]);
        const auto __broadcasted_last_sub = _Simd_(__sub_begin[__sub_length - 1]);

        const auto __sub_bytes = sizeof(_ValueType) * __sub_length;
        const auto __last_offset = __sub_bytes - sizeof(_ValueType);
        const auto __main_bytes = __aligned_size + __tail_size;

        auto __processed_bytes = sizetype(0);

        while (__processed_bytes < __aligned_size && (__main_bytes - __processed_bytes) >= (__last_offset + sizeof(_Simd_))) {
            const auto __loaded_first = datapar::load<_Simd_>(__main_begin);
            const auto __equal_first = (__broadcasted_first_sub == __loaded_first);

            if (datapar::none_of(__equal_first)) {
                __processed_bytes += sizeof(_Simd_);
                __advance_bytes(__main_begin, sizeof(_Simd_));
                continue;
            }

            const auto __loaded_last = datapar::load<_Simd_>(__bytes_pointer_offset(__main_begin, __last_offset));
            const auto __equal_last = (__broadcasted_last_sub == __loaded_last);

            auto __combined = __equal_first & __equal_last;

            if (datapar::any_of(__combined)) {
                auto __combined_mask = datapar::to_mask(__combined);

                do {
                    const auto __trailing_zeros = __combined_mask.count_trailing_zero_bits();

                    const auto __match_bytes = __trailing_zeros * sizeof(_ValueType) + sizeof(_ValueType);
                    const auto __main_match = __bytes_pointer_offset(__main_begin, __match_bytes);

                    if (memcmp(__main_match, __bytes_pointer_offset(__sub_first, sizeof(_ValueType)), __sub_bytes - 2 * sizeof(_ValueType)) == 0)
                        return __main_match - 1;

                    __combined_mask.clear_right_most_set_bit();
                } while (datapar::any_of(__combined_mask));
            }

            __processed_bytes += sizeof(_Simd_);
            __advance_bytes(__main_begin, sizeof(_Simd_));
        }

        const auto __remaining_bytes = __main_bytes - __processed_bytes;
        const auto __remaining_first = __main_begin;

        if (__remaining_bytes < __sub_bytes)
            return __bytes_pointer_offset(__remaining_first, __remaining_bytes);

        return __search_scalar<_ValueType>(__remaining_first,
            __remaining_bytes / sizeof(_ValueType), __sub_first, __sub_length);
    }
};


template <class _Type_>
__raze_simd_algorithm_inline const _Type_* __search_vectorized(
	const void* __main_first,
	sizetype	__main_length,
	const void* __sub_first,
	sizetype	__sub_length) noexcept
{
	return datapar::__simd_sized_dispatcher<__search_vectorized_internal, _Type_>()(
		__main_length * sizeof(_Type_), &__search_scalar<_Type_>, __main_first, __main_length, __sub_first, __sub_length);
}

__RAZE_ALGORITHM_NAMESPACE_END
