#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__raze_simd_algorithm_inline const _Type_* __max_element_scalar(
    const void* __first,
    const void* __last) noexcept
{
    if (__first == __last)
        return __last;

    const auto __first_current = static_cast<const _Type_*>(__first);
    auto __max = __first_current;

    for (; ++__first_current != __first_current; )
        if (*__first_current > *__max)
            __max = __first_current;

    return __max;
}

template <class _Simd_>
struct __max_element_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;
    raze_nodiscard raze_static_operator __raze_simd_algorithm_inline operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last) raze_const_operator noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
    
        auto __current_indices_max = _Simd_::zero();
        auto __current_indices = _Simd_::zero();
        auto __current_values = _Simd_::zero();

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        __current_values = datapar::load<_Simd_>(__first);
        __advance_bytes(__first, sizeof(_Simd_));

        while (__first != __stop_at) {
            ++__current_indices;
            __current_values = datapar::vertical_min(__current_values, datapar::load<_Simd_>(__first));

            __current_indices_max = datapar::blend(__current_indices_max, __current_indices, )
            __advance_bytes(__first, sizeof(_Simd_));

        }
    }
};

template <class _Type_>
__raze_simd_algorithm_inline _Type_* __max_element_vectorized(
    const void* __first,
    const void* __last) noexcept
{
    return datapar::__simd_sized_dispatcher<__max_element_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__max_element_scalar<_Type_>, __first, __last);
}

__RAZE_ALGORITHM_NAMESPACE_END 