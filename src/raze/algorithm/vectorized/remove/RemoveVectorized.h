#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <src/raze/datapar/CachePrefetcher.h>

#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__raze_simd_algorithm_inline _Type_* __remove_scalar(
    void*       __first,
    void*       __current,
    const void* __last,
    _Type_      __value) noexcept
{
    auto* __current_pointer  = static_cast<const _Type_*>(__current);
    auto* __first_pointer    = static_cast<_Type_*>(__first);

    for (; __current_pointer != __last; ++__current_pointer) {
        const auto __current_value = *__current_pointer;

        if (__current_value != __value)
            *__first_pointer++ = __current_value;
    }

    return __first_pointer;
}

template <class _Simd_>
struct __remove_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_static_operator __raze_simd_algorithm_inline _ValueType* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        void*       __first,
        void*       __current,
        const void* __last,
        _ValueType  __value) raze_const_operator noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
        const auto __comparand = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            const auto __loaded = datapar::load<_Simd_>(__current);
            const auto __mask = (__comparand == __loaded);

            __first = datapar::compress_store(__first, __loaded, __mask);
            __advance_bytes(__current, sizeof(_Simd_));
        } while (__current != __stop_at);

        return (__tail_size == 0)
            ? static_cast<_ValueType*>(__first)
            : __remove_scalar<_ValueType>(__first, __current, __last, __value);
    }
};

template <class _Type_>
__raze_simd_algorithm_inline _Type_* __remove_vectorized(
    void*       __first,
    const void* __last,
    _Type_      __value) noexcept
{
    return datapar::__simd_sized_dispatcher<__remove_vectorized_internal, _Type_>()(
        __byte_length(__first, __last), &__remove_scalar<_Type_>, __first, __first, __last, __value);
}

__RAZE_ALGORITHM_NAMESPACE_END
