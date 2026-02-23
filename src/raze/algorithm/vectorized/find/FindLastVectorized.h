#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/datapar/CachePrefetcher.h>

#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline const _Type_* __find_last_scalar(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    auto __current = static_cast<const _Type_*>(__last);

    while (__current != __first && *__current != __value)
        --__current;

    return __current;
}

template <class _Simd_>
struct __find_last_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline const _ValueType* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last,
        _ValueType  __value) const noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();

        const auto __cached_last = __last;
        const auto __comparand = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__last, -__aligned_size);

        do {
            __rewind_bytes(__last, sizeof(_Simd_));

            const auto __loaded  = datapar::load<_Simd_>(__last);
            const auto __mask    = (__comparand == __loaded);

            if (datapar::any_of(__mask)) {
                const auto __index_mask = datapar::to_index_mask(__mask);
                const auto __offset = (__index_mask.elements() - __index_mask.count_leading_zero_bits() - 1);
                return static_cast<const _ValueType*>(__last) + __offset;
            }
        } while (__last != __stop_at);

        if (__tail_size == 0)
            return static_cast<const _ValueType*>(__cached_last);

        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            __rewind_bytes(__last, __tail_size);

            const auto __tail_mask  = datapar::make_tail_mask<_Simd_>(__tail_size);
            const auto __loaded     = datapar::maskz_load<_Simd_>(__last, __tail_mask);

            const auto __mask = (__comparand == __loaded) & __tail_mask;

            if (datapar::any_of(__mask)) {
                const auto __index_mask = datapar::to_index_mask(__mask);
                const auto __offset = (__index_mask.elements() - __index_mask.count_leading_zero_bits() - 1);
                return static_cast<const _ValueType*>(__last) + __offset;
            }

            return static_cast<const _ValueType*>(__cached_last);
        }
        else {
            return __find_last_scalar(__first, __last, __value);
        }
    }
};

template <class _Type_>
__raze_simd_algorithm_inline _Type_* __find_last_vectorized(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    return const_cast<_Type_*>(datapar::__simd_sized_dispatcher<__find_last_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__find_last_scalar<_Type_>, __first, __last, __value));
}
__RAZE_ALGORITHM_NAMESPACE_END