#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>

#include <src/raze/datapar/SizedSimdDispatcher.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_declare_const_function __raze_simd_algorithm_inline _Type_ __max_scalar(
    const void* __first,
    const void* __last) noexcept
{
    const auto* __current = static_cast<const _Type_*>(__first);
    auto __max = __current;

    while (++__current != __last)
        if (*__current > *__max)
            __max = __current;

    return *__max;
}

template <class _Simd_>
struct __max_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_static_operator raze_declare_const_function __raze_simd_algorithm_inline _ValueType operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last) raze_const_operator noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
        auto __maximum_values = _Simd_::zero();

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        __maximum_values = datapar::load<_Simd_>(__first);
        __advance_bytes(__first, sizeof(_Simd_));

        while (__first != __stop_at) {
            __maximum_values = datapar::vertical_max(datapar::load<_Simd_>(__first), __maximum_values);
            __advance_bytes(__first, sizeof(_Simd_));
        }

        if (__tail_size != 0) {
            if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
                const auto __tail_mask = datapar::make_tail_mask<_Simd_>(__tail_size);
                __maximum_values = datapar::vertical_max(__maximum_values, datapar::maskz_load<_Simd_>(__first, __tail_mask));
            }
            else {
                const auto __max            = __max_scalar<_ValueType>(__first, __last);
                const auto __horizontal_max = datapar::horizontal_max(__maximum_values);

                return (__max > __horizontal_max) ? __max : __horizontal_max;
            }
        }

        return datapar::horizontal_max(__maximum_values);
    }
};

template <class _Type_>
raze_declare_const_function __raze_simd_algorithm_inline _Type_ __max_vectorized(
    const void* __first,
    const void* __last) noexcept
{
    return datapar::__simd_sized_dispatcher<__max_vectorized_internal, _Type_>()(
        __byte_length(__first, __last), &__max_scalar<_Type_>, __first, __last);
}

__RAZE_ALGORITHM_NAMESPACE_END