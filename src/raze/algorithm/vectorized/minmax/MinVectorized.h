#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline _Type_ __min_scalar(
    const void* __first,
    const void* __last) noexcept
{
    if (__first == __last)
        return -1;

    const auto* __current = static_cast<const _Type_*>(__first);
    auto __min = __current;

    while (++__current != __last)
        if (*__current < *__min)
            __min = __current;

    return *__min;
}

template <class _Simd_>
struct __min_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline _ValueType operator ()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last) const noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
        auto __minimum_values = _Simd_::zero();

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        __minimum_values = datapar::load<_Simd_>(__first);
        __advance_bytes(__first, sizeof(_Simd_));

        while (__first != __stop_at) {
            __minimum_values = datapar::vertical_min(datapar::load<_Simd_>(__first), __minimum_values);
            __advance_bytes(__first, sizeof(_Simd_));
        }

        if (__tail_size != 0) {
            if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
                const auto __tail_mask = datapar::make_tail_mask<_Simd_>(__tail_size);
                __minimum_values = datapar::vertical_min(__minimum_values, datapar::maskz_load<_Simd_>(__first, __tail_mask));
            }
            else {
                const auto __min            = __min_scalar<_ValueType>(__first, __last);
                const auto __horizontal_min = datapar::horizontal_min(__minimum_values);

                return (__min < __horizontal_min) ? __min : __horizontal_min;
            }
        }

        return datapar::horizontal_min(__minimum_values);
    }
};

template <class _Type_>
raze_always_inline _Type_ __min_vectorized(
    const void* __first,
    const void* __last) noexcept
{
    return datapar::__simd_sized_dispatcher<__min_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__min_scalar<_Type_>, __first, __last);
}

__RAZE_ALGORITHM_NAMESPACE_END