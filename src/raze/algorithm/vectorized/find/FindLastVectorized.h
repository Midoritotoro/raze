#pragma once

#include <raze/vx/SimdDataparAlgorithms.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__raze_simd_algorithm_inline const _Type_* __find_last_scalar(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    auto __current = static_cast<const _Type_*>(__last);
    auto __cached = __current;

    while (__current != __first) {
        if (*__current == __value)
            return __current;

        --__current;
    }

    return __cached;
}

template <class _Simd_>
struct __find_last_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_static_operator __raze_simd_algorithm_inline const _ValueType* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last,
        _ValueType  __value) raze_const_operator noexcept
    {
        const auto __guard = vx::make_guard<_Simd_>();

        const auto __cached_last = __last;
        const auto __comparand = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__last, -__aligned_size);

        do {
            __rewind_bytes(__last, sizeof(_Simd_));

            const auto __loaded  = vx::load<_Simd_>(__last);
            const auto __mask    = (__comparand == __loaded);

            if (vx::any_of(__mask))
                return static_cast<const _ValueType*>(__last) + _Simd_::size() - vx::find_last_set(__mask) - 1;
        } while (__last != __stop_at);

        if (__tail_size == 0)
            return static_cast<const _ValueType*>(__cached_last);

        if constexpr (_Simd_::is_native_mask_load_supported_v) {
            __rewind_bytes(__last, __tail_size);

            const auto __tail_mask  = vx::first_n<_Simd_>(__tail_size / sizeof(_ValueType));
            const auto __loaded     = vx::maskz_load<_Simd_>(__last, __tail_mask);

            const auto __mask = (__comparand == __loaded) & __tail_mask;

            if (vx::any_of(__mask))
                return static_cast<const _ValueType*>(__last) + _Simd_::size() - vx::find_last_set(__mask) - 1;

            return static_cast<const _ValueType*>(__cached_last);
        }
        else {
            const auto __scalar_result = __find_last_scalar(__first, __last, __value);

            if (__scalar_result == __last)
                return static_cast<const _ValueType*>(__cached_last);

            return __scalar_result;
        }
    }
};

template <class _Type_>
__raze_simd_algorithm_inline _Type_* __find_last_vectorized(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    return const_cast<_Type_*>(vx::__simd_sized_dispatcher<__find_last_vectorized_internal, _Type_>()(
        __byte_length(__first, __last), &__find_last_scalar<_Type_>, __first, __last, __value));
}

__RAZE_ALGORITHM_NAMESPACE_END
