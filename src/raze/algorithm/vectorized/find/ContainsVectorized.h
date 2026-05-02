#pragma once

#include <raze/vx/.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_declare_const_function __raze_simd_algorithm_inline bool __contains_scalar(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    auto __current = static_cast<const _Type_*>(__first);

    while (__current != __last) {
        if (*__current == __value)
            return true;

        ++__current;
    }

    return false;
}

template <class _Simd_>
struct __contains_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_declare_const_function raze_static_operator __raze_simd_algorithm_inline bool operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last,
        _ValueType  __value) raze_const_operator noexcept
    {
        const auto __guard      = vx::make_guard<_Simd_>();
        const auto __comparand  = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            if (vx::any_of(vx::load<_Simd_>(__first) == __comparand))
                return true;

            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return false;
        
        if constexpr (_Simd_::is_native_mask_load_supported_v) {
            const auto __tail_mask  = vx::first_n<_Simd_>(__tail_size / sizeof(_ValueType));
            return vx::any_of((__comparand == vx::maskz_load<_Simd_>(__first, __tail_mask)) & __tail_mask);
        }
        else {
            return __contains_scalar(__first, __last, __value);
        }
    }
};

template <class _Type_>
raze_declare_const_function __raze_simd_algorithm_inline bool __contains_vectorized(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    return vx::__simd_sized_dispatcher<__contains_vectorized_internal, _Type_>()(
        __byte_length(__first, __last), &__contains_scalar<_Type_>, __first, __last, __value);
}


__RAZE_ALGORITHM_NAMESPACE_END
