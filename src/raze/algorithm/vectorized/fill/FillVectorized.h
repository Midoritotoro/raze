#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>

#include <raze/memory/Alignment.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <sizetype _Step_>
struct _Memset_step_type {
    using type = void;
};

template <>
struct _Memset_step_type<1> {
    using type = uint8;
};

template <>
struct _Memset_step_type<2> {
    using type = uint16;
};

template <>
struct _Memset_step_type<4> {
    using type = uint32;
};

template <>
struct _Memset_step_type<8> {
    using type = uint64;
};

template <>
struct _Memset_step_type<16> {
    using type = __m128i;
};

template <>
struct _Memset_step_type<32> {
    using type = __m256i;
};

template <>
struct _Memset_step_type<64> {
    using type = __m512i;
};

template <class _Type_>
raze_always_inline void* __memset_scalar(
    void*       __destination,
    _Type_      __value,
    sizetype    __count) noexcept
{
    if (__count == 0)
        return __destination;

    auto* __for_write = static_cast<_Type_*>(__destination);

    do {
        *__for_write++ = __value;
    } while (--__count);

    return __for_write;
}

template <class _Simd_> 
struct __memset_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;

    raze_always_inline void* operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        void*       __destination,
        _ValueType  __value,
        sizetype    __count) const noexcept
    {
        raze_unused(__count);

        const auto __guard = datapar::make_guard<_Simd_>();

        constexpr auto __block_size = sizeof(_Simd_);
        const auto __broadcasted = _Simd_(__value);

        const auto __stop_at = __bytes_pointer_offset(__destination, __aligned_size);

        do {
            datapar::store(__destination, __broadcasted);
            __advance_bytes(__destination, __block_size);
        } while (__destination != __stop_at);

        if (__tail_size == 0)
            return __destination;

        if constexpr (_Simd_::template is_native_mask_store_supported_v<>)
            datapar::mask_store(__destination, __broadcasted, datapar::make_tail_mask<_Simd_>(__tail_size));
        else
            return __memset_scalar(__destination, __value, (__tail_size / sizeof(_ValueType)));

        __advance_bytes(__destination, __tail_size);
        return __destination;
    }
};

template <class _Type_>
raze_always_inline void* __memset_vectorized(
    void*       __destination,
    _Type_      __value,
    sizetype    __count) noexcept
{
    return datapar::__simd_sized_dispatcher<__memset_vectorized_internal>::__apply<_Type_>(
        __count * sizeof(_Type_), &__memset_scalar<_Type_>, __destination, __value, __count);
}

__RAZE_ALGORITHM_NAMESPACE_END
