#pragma once


#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <typename _Type_>
raze_always_inline void __replace_scalar(
    void*           __first,
    void*           __last,
    const _Type_    __old_value,
    const _Type_    __new_value) noexcept
{
    auto __first_pointer = static_cast<_Type_*>(__first);

    for (; __first_pointer != __last; ++__first_pointer)
        if (*__first_pointer == __old_value)
            *__first_pointer = __new_value;
}

template <class _Simd_>
struct __replace_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline void operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        void*       __first,
        void*       __last,
        _ValueType  __old_value,
        _ValueType  __new_value) const noexcept
    {
        constexpr auto __is_masked_memory_access_supported = _Simd_::template is_native_mask_store_supported_v<> &&
            _Simd_::template is_native_mask_load_supported_v<>;

        const auto __guard = datapar::make_guard<_Simd_>();
        const auto __stop_at = __bytes_pointer_offset(__first, __aligned_size);

        const auto __comparand      = _Simd_(__old_value);
        const auto __replacement    = _Simd_(__new_value);

        do {
            const auto __loaded = datapar::load<_Simd_>(__first);
            const auto __mask = (__loaded == __comparand) | datapar::as_native;

            datapar::mask_store(__first, __replacement, __mask);
            __advance_bytes(__first, sizeof(_Simd_));
        } while (__first != __stop_at);

        if (__tail_size == 0)
            return;

        if constexpr (__is_masked_memory_access_supported) {
            const auto __tail_mask  = datapar::make_tail_mask<_Simd_>(__tail_size);
            const auto __loaded     = datapar::maskz_load<_Simd_>(__first, __tail_mask);

            const auto __mask = ((__loaded == __comparand) & __tail_mask) | datapar::as_native;
            datapar::mask_store(__first, __replacement, __mask);
        }
        else {
            __replace_scalar<typename _Simd_::value_type>(__first, __last, __old_value, __new_value);
        }
    }
};

template <typename _Type_>
raze_always_inline void __replace_vectorized(
    void*           __first,
    void*           __last,
    const _Type_    __old_value,
    const _Type_    __new_value) noexcept
{
    datapar::__simd_sized_dispatcher<__replace_vectorized_internal>::__apply<_Type_>(
        __byte_length(__first, __last), &__replace_scalar<_Type_>,
        __first, __last, __old_value, __new_value);
}

__RAZE_ALGORITHM_NAMESPACE_END

