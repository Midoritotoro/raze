#pragma once

#include <src/raze/datapar/SizedSimdDispatcher.h>
#include <raze/datapar/SimdDataparAlgorithms.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
__raze_simd_algorithm_inline void __reverse_scalar(
    void* __first,
    void* __last) noexcept
{
    auto* __first_pointer  = static_cast<_Type_*>(__first);
    auto* __last_pointer   = static_cast<_Type_*>(__last);

    for (; __first_pointer != __last_pointer && __first_pointer != --__last_pointer; ++__first_pointer) {
        _Type_ _Temp = *__last_pointer;

        *__last_pointer  = *__first_pointer;
        *__first_pointer = _Temp;
    }
}

template <class _Simd_>
struct __reverse_vectorized_internal {
    raze_static_operator __raze_simd_algorithm_inline void operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        void*       __first,
        void*       __last) raze_const_operator noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            __rewind_bytes(__last, sizeof(_Simd_));

            const auto __first_loaded = datapar::load<_Simd_>(__first);
            const auto __last_loaded = datapar::load<_Simd_>(__last);

            datapar::store(__last, datapar::reverse(__first_loaded));
            datapar::store(__first, datapar::reverse(__last_loaded));

            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size != 0)
            __reverse_scalar<typename _Simd_::value_type>(__first, __last);
    }
};

template <class _Type_>
__raze_simd_algorithm_inline void __reverse_vectorized(
    void* __first,
    void* __last) noexcept
{
    datapar::__simd_sized_dispatcher<__reverse_vectorized_internal, _Type_>()(
        __byte_length(__first, __last) >> 1, &__reverse_scalar<_Type_>, __first, __last);
}


__RAZE_ALGORITHM_NAMESPACE_END
