#pragma once

#include <raze/vx/SimdDataparAlgorithms.h>
#include <src/raze/vx/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__raze_simd_algorithm_inline const _Type_* __adjacent_find_scalar(
    const void* __first,
    const void* __last) noexcept
{
    auto __first_current = static_cast<const _Type_*>(__first);

    if (__first_current == __last)
        return static_cast<const _Type_*>(__last);

    for (auto __next = __first_current; ++__next != __last; __first_current = __next)
        if (*__first_current == *__next)
            return __first_current;

    return static_cast<const _Type_*>(__last);
}

template <class _Simd_>
struct __adjacent_find_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    __raze_simd_algorithm_inline raze_static_operator const _ValueType* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last) raze_const_operator noexcept
    {
        const auto __guard      = vx::make_guard<_Simd_>();
        const auto __stop_at    = __bytes_pointer_offset(__first, __aligned_size);

        auto __next = __bytes_pointer_offset(__first, sizeof(_ValueType));

        do {
            const auto __loaded_current = vx::load<_Simd_>(__first);
            const auto __loaded_next    = vx::load<_Simd_>(__next);

            const auto __mask = __loaded_current == __loaded_next;

            if (vx::any_of(__mask))
                return static_cast<const _ValueType*>(__first) + vx::find_first_set(__mask);

            __advance_bytes(__first, sizeof(_Simd_));
            __advance_bytes(__next, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return static_cast<const _ValueType*>(__last);

        if constexpr (_Simd_::is_native_mask_load_supported_v) {
            const auto __tail_mask = vx::first_n<_Simd_>(__tail_size / sizeof(_ValueType));

            const auto __loaded_current = vx::maskz_load<_Simd_>(__first, __tail_mask);
            const auto __loaded_next = vx::maskz_load<_Simd_>(__next, __tail_mask);

            const auto __mask = (__loaded_current == __loaded_next) & __tail_mask;

            if (vx::any_of(__mask))
                return static_cast<const _ValueType*>(__first) + vx::find_first_set(__mask);

            __advance_bytes(__first, __tail_size);
        }
        else {
            __last = __adjacent_find_scalar<_ValueType>(__first, __last);
        }

        return static_cast<const _ValueType*>(__last);
    }
};

template <class _Type_>
__raze_simd_algorithm_inline _Type_* __adjacent_find_vectorized(
    const void* __first,
    const void* __last) noexcept
{
    if (__first == __last)
        return const_cast<_Type_*>(static_cast<const _Type_*>(__last));

    return const_cast<_Type_*>(vx::__simd_sized_dispatcher<__adjacent_find_vectorized_internal, _Type_>()(
        __byte_length(__first, __last) - sizeof(_Type_), &__adjacent_find_scalar<_Type_>, __first, __last));
}

__RAZE_ALGORITHM_NAMESPACE_END
