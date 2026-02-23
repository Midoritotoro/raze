#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <typename _Type_>
raze_always_inline void __replace_copy_scalar(
    const void*     __first,
    const void*     __last,
    void*           __destination,
    const _Type_    __old_value,
    const _Type_    __new_value) noexcept
{
    auto* __current              = static_cast<const _Type_*>(__first);
    auto* __destination_current  = static_cast<_Type_*>(__destination);

    for (; __current != __last; ++__current, ++__destination_current)
        *__destination_current = (*__current == __old_value) ? __new_value : (*__current);
}


template <class _Simd_>
struct __replace_copy_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline void operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last,
        void*       __destination,
        _ValueType  __old_value,
        _ValueType  __new_value) noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        const auto __comparand = _Simd_(__old_value);
        const auto __replacement = _Simd_(__new_value);

       do {
            const auto __loaded = datapar::load<_Simd_>(__first);
            const auto __native_mask = (__loaded == __comparand);

            datapar::mask_store(__destination, __replacement, __native_mask);

            __advance_bytes(__first, sizeof(_Simd_));
            __advance_bytes(__destination, sizeof(_Simd_));
       } while (__first != __stop_at);

       if (__tail_size == 0)
           return;

        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            const auto __tail_mask = datapar::make_tail_mask<_Simd_>(__tail_size);
            const auto __loaded = datapar::maskz_load<_Simd_>(__first, __tail_mask);

            const auto __mask = ((__loaded == __comparand) & __tail_mask);
            datapar::mask_store(__destination, __replacement, __mask);
        }
        else {
            __replace_copy_scalar<_ValueType>(__first, __last, __destination, __old_value, __new_value);
        }
    }
};

template <typename _Type_>
raze_always_inline void __replace_copy_vectorized(
    const void*     __first,
    const void*     __last,
    void*           __destination,
    const _Type_    __old_value,
    const _Type_    __new_value) noexcept
{
    datapar::__simd_sized_dispatcher<__replace_copy_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__replace_copy_scalar<_Type_>,
        __first, __last, __destination, __old_value, __new_value);
}

__RAZE_ALGORITHM_NAMESPACE_END

