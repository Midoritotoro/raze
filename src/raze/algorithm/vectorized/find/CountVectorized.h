#pragma once

#include <raze/vx/SimdDataparAlgorithms.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_declare_const_function __raze_simd_algorithm_inline sizetype __count_scalar(
    const void*     __first,
    const sizetype  __bytes,
    _Type_          __value) noexcept
{
    auto __current      = static_cast<const _Type_*>(__first);
    const auto __length = __bytes / sizeof(_Type_);

    auto __count = sizetype(0);

    for (auto __index = sizetype(0); __index < __length; ++__index)
        __count += (*__current++ == __value);

    return __count;
}


template <class _Simd_>
struct __count_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_declare_const_function raze_static_operator __raze_simd_algorithm_inline sizetype operator()(
        sizetype                                __aligned_size,
        sizetype                                __tail_size,
        const void*                             __first,
        raze_maybe_unused_attribute sizetype    __bytes,
        _ValueType                              __value) raze_const_operator noexcept
    {
        const auto __guard = vx::make_guard<_Simd_>();
        const auto __comparand = _Simd_(__value);

        auto __result = uint64(0);
        auto __counter = vx::simd_counter<_Simd_>();

        constexpr auto __max_portion_size = vx::simd_counter<_Simd_>::portion_size();

        while (true) {
            const auto __current_portion_size = raze::algorithm::min(__aligned_size, __max_portion_size);
            const auto __stop_at = __bytes_pointer_offset(__first, __current_portion_size);

            __aligned_size -= __current_portion_size;

            do {
                __counter.add(vx::load<_Simd_>(__first) == __comparand);
                __advance_bytes(__first, sizeof(_Simd_));
            } while (__first != __stop_at);

            __result += __counter.finalize();

            if (__aligned_size == 0)
                break;
        }

        if constexpr (_Simd_::is_native_mask_load_supported_v) {
            if (__tail_size != 0) {
                const auto __tail_mask = vx::first_n<_Simd_>(__tail_size / sizeof(_ValueType));
                __counter.add(vx::maskz_load<_Simd_>(__first, __tail_mask) == __comparand, __tail_mask);
            }

            return __result + __counter.finalize();
        } 
        else {
            return __result + __count_scalar(__first, __tail_size, __value);
        }
    }
};

template <class _Type_>
raze_declare_const_function __raze_simd_algorithm_inline sizetype __count_vectorized(
    const void*     __first,
    const sizetype  __bytes,
    _Type_          __value) noexcept
{
    return vx::__simd_sized_dispatcher<__count_vectorized_internal, _Type_>()(
        __bytes, &__count_scalar<_Type_>, __first, __bytes, __value);
}

__RAZE_ALGORITHM_NAMESPACE_END
