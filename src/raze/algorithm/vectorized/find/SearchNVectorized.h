#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    class _DifferenceType_,
    class _Type_>
__raze_simd_algorithm_inline const _Type_* __search_n_scalar(
    const void*         __first,
    const void*         __last,
    _DifferenceType_    __count,
    _Type_              __value) noexcept
{
    auto __current = static_cast<const _Type_*>(__first);

    while (__current != __last && *__current != __value)
        ++__current;

    return __current;
}

template <class _Simd_>
struct __search_n_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    template <class _DifferenceType_>
    __raze_simd_algorithm_inline raze_static_operator const typename _Simd_::value_type* operator()(
        sizetype                    __aligned_size,
        sizetype                    __tail_size,
        const void*                 __first,
        const void*                 __last,
        _DifferenceType_            __count,
        typename _Simd_::value_type __value) raze_const_operator noexcept
    {
        const auto __guard = datapar::make_guard<_Simd_>();
        const auto __comparand = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);
        auto __current_count = _DifferenceType_(0);

        do {
            const auto __loaded = datapar::load<_Simd_>(__first);
            const auto __mask = __loaded == __comparand;

            if (datapar::any_of(__mask)) {
                const auto __bits = __mask.bits();
              

            }

            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return static_cast<const _ValueType*>(__last);

        if constexpr (_Simd_::is_native_mask_load_supported_v) {
            const auto __tail_mask = datapar::first_n<_Simd_>(__tail_size / sizeof(_ValueType));
        }
        else {
            __last = __search_n_scalar(__first, __last, __value);
        }

        return static_cast<const _ValueType*>(__last);
    }
};

template <
    class _DifferenceType_,
    class _Type_>
__raze_simd_algorithm_inline _Type_* __search_n_vectorized(
    const void*         __first,
    const void*         __last,
    _DifferenceType_    __count,
    _Type_              __value) noexcept
{
    return const_cast<_Type_*>(datapar::__simd_sized_dispatcher<__search_n_vectorized_internal, _Type_>()(
        __byte_length(__first, __last), &__search_n_scalar<_Type_>, __first, __last, __count, __value));
}

__RAZE_ALGORITHM_NAMESPACE_END
