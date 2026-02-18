#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline std::pair<_Type_, _Type_> __minmax_scalar(
    const void* __first,
    const void* __last) noexcept
{
    const auto* __current = static_cast<const _Type_*>(__first);

    auto __max = __current;
    auto __min = __current;

    while (++__current != __last) {
        if (*__current > *__max)
            __max = __current;
        else if (*__current < *__min)
            __min = __current;
    }

    return { *__min, *__max };
}

template <class _Simd_>
struct __minmax_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline std::pair<_ValueType, _ValueType> operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last) noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();

        auto __maximum_values = _Simd_::zero();
        auto __minimum_values = _Simd_::zero();

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        __maximum_values = datapar::load<_Simd_>(__first);
        __advance_bytes(__first, sizeof(_Simd_));

        while (__first != __stop_at) {
            const auto __loaded = datapar::load<_Simd_>(__first);

            __maximum_values = datapar::vertical_max(__maximum_values, __loaded);
            __minimum_values = datapar::vertical_min(__minimum_values, __loaded);

            __advance_bytes(__first, sizeof(_Simd_));
        }

        if (__tail_size != 0) {
            if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
                const auto __loaded = datapar::maskz_load<_Simd_>(__first, datapar::make_tail_mask<_Simd_>(__tail_size));

                __maximum_values = datapar::vertical_max(__maximum_values, __loaded);
                __minimum_values = datapar::vertical_min(__minimum_values, __loaded);
            }
            else {
                const auto __minmax = __minmax_scalar<_ValueType>(__first, __last);

                return std::pair{
                    std::min(__minmax.first, datapar::horizontal_min(__minimum_values)),
                    std::max(__minmax.second, datapar::horizontal_max(__maximum_values))
                };
            }
        }

        return { datapar::horizontal_min(__minimum_values), datapar::horizontal_max(__maximum_values) };
    }
};

template <class _Type_>
raze_always_inline std::pair<_Type_, _Type_> __minmax_vectorized(
    const void* __first,
    const void* __last) noexcept
{
    return datapar::__simd_sized_dispatcher<__minmax_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__minmax_scalar<_Type_>, __first, __last);
}

__RAZE_ALGORITHM_NAMESPACE_END