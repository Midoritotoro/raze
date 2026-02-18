#pragma once

#include <raze/datapar/BasicSimd.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <
    typename    _Type_,
    class       _UnaryPredicate_>
raze_always_inline void* raze_stdcall _TransformScalar(
    const void*         _First,
    const void*         _Last,
    void*               _Destination,
    _UnaryPredicate_    _Predicate) noexcept
{
    auto _FirstCurrent          = static_cast<const _Type_*>(_First);
    auto _DestinationCurrent    = static_cast<_Type_*>(_Destination);

    for (; _FirstCurrent != _Last; ++_FirstCurrent, ++_DestinationCurrent)
        *_DestinationCurrent = _Predicate(*_FirstCurrent);

    return _DestinationCurrent;
}

template <
    typename    _Type_,
    class       _BinaryPredicate_>
raze_always_inline void* raze_stdcall _TransformScalar(
    const void*         _First1,
    const void*         _Last1,
    const void*         _First2,
    void*               _Destination,
    _BinaryPredicate_   _Predicate) noexcept
{
    auto _First1Current = static_cast<const _Type_*>(_First1);
    auto _First2Current = static_cast<const _Type_*>(_First2);

    auto _DestinationCurrent = static_cast<_Type_*>(_Destination);

    for (; _First1Current != _Last1; ++_First1Current, ++_First2Current, ++_DestinationCurrent)
        *_DestinationCurrent = _Predicate(*_First1Current, *_First2Current);

    return _DestinationCurrent;
}

template <
    arch::ISA    _SimdGeneration_,
    typename            _Type_,
    class               _UnaryPredicate_>
raze_always_inline void* raze_stdcall _TransformVectorizedInternal(
    const void*         _First,
    const void*         _Last,
    void*               _Destination,
    _UnaryPredicate_    _Predicate) noexcept
{
    using _SimdType_ = datapar::simd<_SimdGeneration_, _Type_>;

    if constexpr (_Is_predicate_vectorizable_v<_UnaryPredicate_, _SimdType_> == false) {
        return _TransformScalar<_Type_>(_First, _Last, _Destination, type_traits::passFunction(_Predicate));
    }
    else {
        constexpr auto _Is_masked_memory_access_supported = _SimdType_::template is_native_mask_store_supported_v<> &&
            _SimdType_::template is_native_mask_load_supported_v<>;

        const auto _Size = __byte_length(_First, _Last);
        const auto _AlignedSize = _Size & (~(sizeof(_SimdType_) - 1));

        const void* _StopAt = _First;
        __advance_bytes(_StopAt, _AlignedSize);

        while (_First != _StopAt) {
            const auto _Applied = _Predicate(_SimdType_::loadUnaligned(_First));
            _Applied.storeUnaligned(_Destination);

            __advance_bytes(_Destination, sizeof(_SimdType_));
            __advance_bytes(_First, sizeof(_SimdType_));
        }

        if constexpr (_Is_masked_memory_access_supported) {
            const auto _TailSize = _Size & (sizeof(_SimdType_) - sizeof(_Type_));

            if (_TailSize != 0) {
                const auto _TailMask    = _SimdType_::makeTailMask(_TailSize);
                const auto _Loaded      = _SimdType_::maskLoadUnaligned(_First, _TailMask);

                const auto _Applied     = _Predicate(_Loaded);
                _Applied.maskStoreUnaligned(_Destination, _TailMask);
            }
        }
        else {
            if (_First != _Last)
                _Destination = _TransformScalar<_Type_>(_First, _Last, _Destination, type_traits::passFunction(_Predicate));
        }

        _SimdType_::zeroUpper();
    }

    return _Destination;
}

template <
    arch::ISA    _SimdGeneration_,
    typename            _Type_,
    class               _BinaryPredicate_>
raze_always_inline void* raze_stdcall _TransformVectorizedInternal(
    const void*         _First1,
    const void*         _Last1,
    const void*         _First2,
    void*               _Destination,
    _BinaryPredicate_   _Predicate) noexcept
{
    using _SimdType_ = datapar::simd<_SimdGeneration_, _Type_>;

    if constexpr (_Is_predicate_vectorizable_v<_BinaryPredicate_, _SimdType_>) {
        constexpr auto _Is_masked_memory_access_supported = _SimdType_::template is_native_mask_store_supported_v<> &&
            _SimdType_::template is_native_mask_load_supported_v<>;

        const auto _Size = __byte_length(_First1, _Last1);
        const auto _AlignedSize = _Size & (~(sizeof(_SimdType_) - 1));

        const void* _StopAt = _First1;
        __advance_bytes(_StopAt, _AlignedSize);

        while (_First1 != _StopAt) {
            const auto _Loaded1 = _SimdType_::loadUnaligned(_First1);
            const auto _Loaded2 = _SimdType_::loadUnaligned(_First2);

            const auto _Applied = _Predicate(_Loaded1, _Loaded2);
            _Applied.storeUnaligned(_Destination);

            __advance_bytes(_First1, sizeof(_SimdType_));
            __advance_bytes(_First2, sizeof(_SimdType_));
            __advance_bytes(_Destination, sizeof(_SimdType_));
        }

        if (_First1 != _Last1)
            _Destination = _TransformScalar<_Type_>(_First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate));
    }

    return _TransformScalar<_Type_>(_First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate));
}

template <
    typename    _Type_,
    class       _UnaryPredicate_>
_Type_* raze_stdcall _TransformVectorized(
    const void*         _First,
    const void*         _Last,
    void*               _Destination,
    _UnaryPredicate_    _Predicate) noexcept
{
    if constexpr (sizeof(_Type_) <= 2) {
        if (arch::ProcessorFeatures::AVX512BW())
            return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::AVX512BW, _Type_>(
                _First, _Last, _Destination, type_traits::passFunction(_Predicate)));
    }
    else {
        if (arch::ProcessorFeatures::AVX512F())
            return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::AVX512F, _Type_>(
                _First, _Last, _Destination, type_traits::passFunction(_Predicate)));
    }

    if (arch::ProcessorFeatures::AVX2())
        return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::AVX2, _Type_>(
            _First, _Last, _Destination, type_traits::passFunction(_Predicate)));
    else if (arch::ProcessorFeatures::SSE41())
        return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::SSE41, _Type_>(
            _First, _Last, _Destination, type_traits::passFunction(_Predicate)));
    else if (arch::ProcessorFeatures::SSE2())
        return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::SSE2, _Type_>(
            _First, _Last, _Destination, type_traits::passFunction(_Predicate)));

    return static_cast<_Type_*>(_TransformScalar<_Type_>(
        _First, _Last, _Destination, type_traits::passFunction(_Predicate)));
}

template <
    typename    _Type_,
    class       _BinaryPredicate_>
_Type_* raze_stdcall _TransformVectorized(
    const void*         _First1,
    const void*         _Last1,
    const void*         _First2,
    void*               _Destination,
    _BinaryPredicate_   _Predicate) noexcept
{
    if constexpr (sizeof(_Type_) <= 2) {
        if (arch::ProcessorFeatures::AVX512BW())
            return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::AVX512BW, _Type_>(
                _First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate)));
    }
    else {
        if (arch::ProcessorFeatures::AVX512F())
            return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::AVX512F, _Type_>(
                _First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate)));
    }

    if (arch::ProcessorFeatures::AVX2())
        return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::AVX2, _Type_>(
            _First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate)));
    else if (arch::ProcessorFeatures::SSE41())
        return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::SSE41, _Type_>(
            _First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate)));
    else if (arch::ProcessorFeatures::SSE2())
        return static_cast<_Type_*>(_TransformVectorizedInternal<arch::ISA::SSE2, _Type_>(
            _First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate)));

    return static_cast<_Type_*>(_TransformScalar<_Type_>(
        _First1, _Last1, _First2, _Destination, type_traits::passFunction(_Predicate)));
}
__RAZE_ALGORITHM_NAMESPACE_END

