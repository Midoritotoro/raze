#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/datapar/CachePrefetcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <typename _Type_> 
raze_declare_const_function __raze_simd_algorithm_inline sizetype __mismatch_scalar(
    const void* __first,
    const void* __second,
    sizetype    __length) noexcept
{
    const _Type_* __first_pointer   = static_cast<const _Type_*>(__first);
    const _Type_* __second_pointer  = static_cast<const _Type_*>(__second);

    while (__length--)
        if (*__first_pointer++ != *__second_pointer++)
            return (__first_pointer - static_cast<const _Type_*>(__first));
   
    return (__first_pointer - static_cast<const _Type_*>(__first));
}

template <class _Simd_>
struct __mismatch_vectorized_internal {
    raze_static_operator raze_declare_const_function __raze_simd_algorithm_inline sizetype operator()(
        sizetype            __aligned_size,
        sizetype            __tail_size,
        const void*         __first,
        const void*         __second,
        const sizetype      __length) raze_const_operator noexcept
    {
        using _ValueType = typename _Simd_::value_type;

        const auto __guard = datapar::make_guard<_Simd_>();
        auto __cached_first = static_cast<const _ValueType*>(__first);

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            const auto __loaded_first   = datapar::load<_Simd_>(__first);
            const auto __loaded_second  = datapar::load<_Simd_>(__second);

            const auto __compared = (__loaded_first == __loaded_second);

            if (datapar::some_of(__compared)) {
                const auto __index_mask = datapar::to_index_mask(__compared);
                return (static_cast<const _ValueType*>(__first) - __cached_first) + __index_mask.count_trailing_one_bits();
            }

            __advance_bytes(__first, sizeof(_Simd_));
            __advance_bytes(__second, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return __length;

        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            const auto __tail_mask = datapar::make_tail_mask<_Simd_>(__tail_size);

            const auto __loaded_first   = datapar::maskz_load<_Simd_>(__first, __tail_mask);
            const auto __loaded_second  = datapar::maskz_load<_Simd_>(__second, __tail_mask);

            const auto __combined_mask = datapar::to_mask((__loaded_first == __loaded_second) & __tail_mask);

            const auto __tail_length    = (__tail_size / sizeof(_ValueType));
            const auto __all_equal_mask = (typename _Simd_::mask_type::mask_type(1) << __tail_length) - 1;

            if (__combined_mask != __all_equal_mask)
                return (static_cast<const _ValueType*>(__first) - __cached_first) + __combined_mask.count_trailing_one_bits();

            return __length;
        }
        else {
            return (__aligned_size / sizeof(_ValueType)) + __mismatch_scalar<_ValueType>(__first, __second, __tail_size / sizeof(_ValueType));
        }
    }
};

template <typename _Type_>
raze_declare_const_function __raze_simd_algorithm_inline sizetype __mismatch_vectorized(
    const void*     __first,
    const void*     __second,
    const sizetype  __length) noexcept
{
    return datapar::__simd_sized_dispatcher<__mismatch_vectorized_internal, _Type_>()(
        __length * sizeof(_Type_), &__mismatch_scalar<_Type_>, __first, __second, __length);
}

__RAZE_ALGORITHM_NAMESPACE_END
