#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/datapar/CachePrefetcher.h>

#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline sizetype __count_scalar(
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
    raze_always_inline sizetype operator()(
        sizetype                    __aligned_size,
        sizetype                    __tail_size,
        const void*                 __first,
        typename _Simd_::value_type __value) noexcept
    {
        const auto __guard  = datapar::make_guard<_Simd_>();
        auto __count        = sizetype(0);

        const auto __comparand  = _Simd_(__value);
        const auto __stop_at    = __bytes_pointer_offset(__first, __aligned_size);

        do {
            __count += datapar::reduce_equal(__comparand, datapar::load<_Simd_>(__first));
            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            if (__tail_size != 0) {
                const auto __tail_mask  = datapar::make_tail_mask<_Simd_>(__tail_size);
                __count += datapar::reduce_equal(datapar::maskz_load<_Simd_>(__first, __tail_mask), __comparand, __tail_mask);
            }

            return __count;
        } 
        else {
            return __count + __count_scalar(__first, __tail_size, __value);
        }
    }
};

template <class _Type_>
raze_always_inline sizetype __count_vectorized(
    const void*     __first,
    const sizetype  __bytes,
    _Type_          __value) noexcept
{
    const auto __fallback_args  = std::forward_as_tuple(__first, __bytes, __value);
    const auto __simd_args      = std::forward_as_tuple(__first, __value);

    return datapar::__simd_sized_dispatcher<__count_vectorized_internal>::__apply<_Type_>(
        __bytes, &__count_scalar<_Type_>, std::move(__simd_args), std::move(__fallback_args));
}

__RAZE_ALGORITHM_NAMESPACE_END
