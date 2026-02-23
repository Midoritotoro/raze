#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline _Type_* __remove_copy_scalar(
    const void* __first,
    const void* __last,
    void*       __destination,
    _Type_      __value) noexcept
{
    auto __first_pointer       = static_cast<const _Type_*>(__first);
    auto __destination_pointer = static_cast<_Type_*>(__destination);

    for (; __first_pointer != __last; ++__first_pointer) {
        const auto __current_value = *__first_pointer;

        if (__current_value != __value)
            *__destination_pointer++ = __current_value;
    }

    return __destination_pointer;
}

template <class _Simd_>
struct __remove_copy_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline _ValueType* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last,
        void*       __destination,
        _ValueType  __value) noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);
        const auto __comparand = _Simd_(__value);

        do {
            const auto __loaded = datapar::load<_Simd_>(__first);
            const auto __mask = (__loaded == __comparand);
            
            __destination = datapar::compress_store(__destination, __loaded, __mask);
            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        return (__first == __last)
            ? static_cast<_ValueType*>(__destination)
            : __remove_copy_scalar(__first, __last, __destination, __value);
    }
};

template <class _Type_>
raze_always_inline _Type_* __remove_copy_vectorized(
    const void* __first,
    const void* __last,
    void*       __destination,
    _Type_      __value) noexcept
{
    return datapar::__simd_sized_dispatcher<__remove_copy_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__remove_copy_scalar<_Type_>,
        __first, __last, __destination, __value);
}


__RAZE_ALGORITHM_NAMESPACE_END
