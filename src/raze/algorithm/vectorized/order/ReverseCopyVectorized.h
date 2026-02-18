#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline void __reverse_copy_scalar(
    const void* __first,
    const void* __last,
    void*       __destination) noexcept
{
    auto* __first_pointer = static_cast<const _Type_*>(__first);
    auto* __last_pointer = static_cast<const _Type_*>(__last);

    auto* __destination_pointer  = static_cast<_Type_*>(__destination);

    for (; __first_pointer != __last_pointer; ++__destination_pointer)
        *__destination_pointer = *--__last_pointer;
}

template <class _Simd_>
struct __reverse_copy_vectorized_internal {
    raze_always_inline void operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last,
        void*       __destination) noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
        const auto __stop_at = __bytes_pointer_offset(__last, -__aligned_size);

        do {
            datapar::store(__destination, datapar::reverse(
                datapar::load<_Simd_>(__bytes_pointer_offset(__last, -sizeof(_Simd_)))));

            __advance_bytes(__destination, sizeof(_Simd_));
            __rewind_bytes(__last, sizeof(_Simd_));
        } while (__last != __stop_at);

        if (__first != __last)
            __reverse_copy_scalar<typename _Simd_::value_type>(__first, __last, __destination);
    }
};

template <class _Type_>
raze_always_inline void __reverse_copy_vectorized(
    const void* __first,
    const void* __last,
    void*       __destination) noexcept
{
    return datapar::__simd_sized_dispatcher<__reverse_copy_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__reverse_copy_scalar<_Type_>, __first, __last, __destination);
}


__RAZE_ALGORITHM_NAMESPACE_END
