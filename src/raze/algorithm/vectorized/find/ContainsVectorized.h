#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/datapar/CachePrefetcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_always_inline bool __contains_scalar(
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
    raze_always_inline bool operator()(
        sizetype                    __aligned_size,
        sizetype                    __tail_size,
        const void*                 __first,
        const void*                 __last,
        typename _Simd_::value_type __value) const noexcept
    {
        const auto __guard      = datapar::make_guard<_Simd_>();
        const auto __comparand  = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            const auto __loaded = datapar::load<_Simd_>(__first);

            if (static_cast<bool>(__loaded == __comparand))
                return true;

            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return false;
        
        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            const auto __tail_mask  = datapar::make_tail_mask<_Simd_>(__tail_size);
            const auto __loaded     = datapar::maskz_load<_Simd_>(__first, __tail_mask);

            return static_cast<bool>((__comparand == __loaded) & __tail_mask);
        }
        else {
            return __contains_scalar(__first, __last, __value);
        }
    }
};

template <class _Type_>
raze_always_inline bool __contains_vectorized(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    return datapar::__simd_sized_dispatcher<__contains_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__contains_scalar<_Type_>, __first, __last, __value);
}


__RAZE_ALGORITHM_NAMESPACE_END
