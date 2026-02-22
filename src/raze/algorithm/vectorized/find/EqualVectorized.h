#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/datapar/CachePrefetcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <typename _Type_> 
raze_always_inline bool __equal_scalar(
    const void* __first,
    const void* __second,
    sizetype    __size) noexcept
{
    __size /= sizeof(_Type_);

    auto __first_pointer    = static_cast<const _Type_*>(__first);
    auto __second_pointer   = static_cast<const _Type_*>(__second);

    while (__size--)
        if (*__first_pointer++ != *__second_pointer++)
            return false;

    return true;
}

template <class _Simd_>
struct __equal_vectorized_internal {
    raze_always_inline bool operator()(
        sizetype            __aligned_size,
        sizetype            __tail_size,
        const void*         __first,
        const void*         __second,
        const sizetype      __size) noexcept
    {
        using _ValueType = typename _Simd_::value_type;

        const auto __guard = datapar::make_guard<_Simd_>();
        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            const auto __loaded_first   = datapar::load<_Simd_>(__first);
            const auto __loaded_second  = datapar::load<_Simd_>(__second);

            if (datapar::some_of(__loaded_first == __loaded_second))
                return false;

            __advance_bytes(__first, sizeof(_Simd_));
            __advance_bytes(__second, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return true;

        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            const auto __tail_mask = datapar::make_tail_mask<_Simd_>(__tail_size);

            const auto __loaded_first   = datapar::maskz_load<_Simd_>(__first, __tail_mask);
            const auto __loaded_second  = datapar::maskz_load<_Simd_>(__second, __tail_mask);

            const auto __combined_mask = ((__loaded_first == __loaded_second) & __tail_mask) | datapar::as_index_mask;

            const auto __tail_length = __tail_size / sizeof(_ValueType);
            const auto __all_equal_mask = (typename _Simd_::mask_type::mask_type(1) << __tail_length) - 1;

            return (__combined_mask == __all_equal_mask);
        }
        else {
            return __equal_scalar<_ValueType>(__first, __second, __tail_size);
        }
    }
};

template <typename _Type_>
__raze_simd_algorithm_inline bool __equal_vectorized(
    const void*     __first,
    const void*     __second,
    const sizetype  __size) noexcept
{
    const auto __bytes = __size * sizeof(_Type_);

    return datapar::__simd_sized_dispatcher<__equal_vectorized_internal>::__apply<_Type_>(
        __bytes, &__equal_scalar<_Type_>, __first, __second, __bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
