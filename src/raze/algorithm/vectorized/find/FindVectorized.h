#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>

#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_nodiscard raze_always_inline const _Type_* __find_scalar(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    auto __current = static_cast<const _Type_*>(__first);

    while (__current != __last && *__current != __value)
        ++__current;

    return __current;
}

template <class _Simd_>
struct __find_vectorized_internal {
    raze_nodiscard raze_always_inline raze_static_operator const typename _Simd_::value_type* operator()(
        sizetype                    __aligned_size,
        sizetype                    __tail_size,
        const void*                 __first,
        const void*                 __last,
        typename _Simd_::value_type __value) raze_const_operator noexcept
    {
        using _ValueType = typename _Simd_::value_type;

        const auto __guard      = datapar::make_guard<_Simd_>();
        const auto __comparand  = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        do {
            const auto __loaded = datapar::load<_Simd_>(__first);
            const auto __mask   = __comparand == __loaded;

            if (datapar::any_of(__mask))
                return static_cast<const _ValueType*>(__first) + datapar::to_index_mask(__mask).count_trailing_zero_bits();

            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return static_cast<const _ValueType*>(__last);

        if constexpr (_Simd_::template is_native_mask_load_supported_v<>) {
            const auto __tail_mask  = datapar::make_tail_mask<_Simd_>(__tail_size);
            const auto __loaded     = datapar::maskz_load<_Simd_>(__first, __tail_mask);

            const auto __mask = (__comparand == __loaded) & __tail_mask;

            if (datapar::any_of(__mask))
                return static_cast<const _ValueType*>(__first) + datapar::to_index_mask(__mask).count_trailing_zero_bits();
        }
        else {
            __last = __find_scalar(__first, __last, __value);
        }

        return static_cast<const _ValueType*>(__last);
    }
};

template <class _Type_>
__raze_simd_algorithm_inline raze_nodiscard _Type_* __find_vectorized(
    const void* __first,
    const void* __last,
    _Type_      __value) noexcept
{
    return const_cast<_Type_*>(datapar::__simd_sized_dispatcher<__find_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__find_scalar<_Type_>, __first, __last, __value));
}

__RAZE_ALGORITHM_NAMESPACE_END
