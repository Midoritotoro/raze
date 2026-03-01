#pragma once

#include <raze/datapar/SimdDataparAlgorithms.h>
#include <src/raze/datapar/SizedSimdDispatcher.h>

#include <raze/algorithm/minmax/Min.h>
#include <raze/algorithm/minmax/Max.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_>
__raze_simd_algorithm_inline std::pair<const _Type_ *, const _Type_*> __minmax_element_scalar(
    const void* __first,
    const void* __last) noexcept
{
    if (__first == __last)
        return { static_cast<const _Type_*>(__last), static_cast<const _Type_*>(__last) };

    auto __first_current = static_cast<const _Type_*>(__first);
    auto __minmax = std::pair<const _Type_*, const _Type_*> { __first_current, __first_current };

    for (; ++__first_current != __last; ) {
        if (*__first_current < *__minmax.first)
            __minmax.first = __first_current;
        if (*__first_current > *__minmax.second)
            __minmax.second = __first_current;
    }
    
    return __minmax;
}

template <class _Simd_>
struct __minmax_element_vectorized_internal {
    using _ValueType = typename _Simd_::value_type;
    using _MinmaxType = std::pair<const _ValueType*, const _ValueType*>;

    raze_static_operator __raze_simd_algorithm_inline _MinmaxType operator()(
        sizetype    __aligned_size,
        sizetype    __tail_size,
        const void* __first,
        const void* __last) raze_const_operator noexcept
    {
        using _UnsignedValueType = IntegerForSizeof<_ValueType>::Unsigned;
        using _IndexSimdType = datapar::simd<_Simd_::__isa, _UnsignedValueType, _Simd_::__width>;

        const auto __guard = datapar::make_guard<_Simd_>();

        const auto __first_begin = static_cast<const _ValueType*>(__first);
        auto __minmax = _MinmaxType { __first_begin, __first_begin };

        auto __current_indices_min = _IndexSimdType::zero();
        auto __current_indices_max = _IndexSimdType::zero();
        auto __current_indices = _IndexSimdType::zero();
        auto __current_values = datapar::load<_Simd_>(__first);
        auto __current_values_max = __current_values;
        auto __current_values_min = __current_values;

        constexpr auto __integer_max = math::__maximum_integral_limit<_UnsignedValueType>();
        constexpr auto __max_portion_size = raze::algorithm::max(static_cast<uint64>(__integer_max)
            + 1, static_cast<uint64>(__integer_max)) * sizeof(_Simd_);

        constexpr auto __has_portion_max_value = (math::__maximum_integral_limit<uint64>() != __max_portion_size);
        auto __aligned_portion_size = raze::algorithm::min(__max_portion_size, __aligned_size);

        auto __portion_begin = __first_begin;
        auto __stop_at = __bytes_pointer_offset(__first, __aligned_portion_size);
        __aligned_size -= __aligned_portion_size;

        while (true) {
            __advance_bytes(__first, sizeof(_Simd_));
            ++__current_indices;

            if (__first != __stop_at) {
                __current_values = datapar::load<_Simd_>(__first);

                const auto __greater_mask = (__current_values < __current_values_max);
                const auto __less_mask = (__current_values < __current_values_min);

                __current_indices_min = datapar::blend(__current_indices, __current_indices_min, __less_mask);
                __current_values_min = datapar::blend(__current_values, __current_values_min, __less_mask);

                __current_indices_max = datapar::blend(__current_indices_max, __current_indices, __greater_mask);
                __current_values_max = datapar::vertical_max(__current_values, __current_values_max);
            }
            else {
                const auto __all_max = _Simd_(datapar::horizontal_max(__current_values_max));
                const auto __all_min = _Simd_(datapar::horizontal_min(__current_values_min));

                const auto __native_mask_max = (__current_values_max == __all_max);
                const auto __native_mask_min = (__current_values_min == __all_min);

                const auto __max_values_indices = datapar::blend(__current_indices_max, _IndexSimdType::zero(), __native_mask_max);
                const auto __min_values_indices = datapar::blend(__current_indices_min, _IndexSimdType(_UnsignedValueType(-1)), __native_mask_min);

                const auto __all_max_indices = _IndexSimdType(datapar::horizontal_max(datapar::simd_cast<_UnsignedValueType>(__max_values_indices)));
                const auto __all_min_indices = _IndexSimdType(datapar::horizontal_min(datapar::simd_cast<_UnsignedValueType>(__min_values_indices)));

                const auto __final_max_mask = datapar::to_mask(_Simd_(__all_max_indices == __max_values_indices) & __native_mask_max);
                const auto __final_min_mask = datapar::to_mask(__all_min_indices == __min_values_indices);

                const auto __horizontal_max_position = (__final_max_mask.count_leading_zero_bits() - (sizeof(_ValueType) - 1));
                const auto __horizontal_min_position = __final_min_mask.count_trailing_zero_bits();

                const auto __vertical_max_position = sizetype(__current_indices_max.extract(__horizontal_max_position));
                const auto __vertical_min_position = sizetype(__current_indices_min.extract(__horizontal_min_position));

                const auto __maybe_max_element = __bytes_pointer_offset(__portion_begin,
                    __vertical_max_position * sizeof(_Simd_) + __horizontal_max_position * sizeof(_ValueType));

                const auto __maybe_min_element = __bytes_pointer_offset(__portion_begin,
                    __vertical_min_position * sizeof(_Simd_) + __horizontal_min_position * sizeof(_ValueType));

                if (*__maybe_max_element > *__minmax.second)
                    __minmax.second = __maybe_max_element;
                if (*__maybe_min_element < *__minmax.first)
                    __minmax.first = __maybe_min_element;

                if constexpr (__has_portion_max_value) {
                    __aligned_portion_size = raze::algorithm::min(__max_portion_size, __aligned_size);

                    if (__aligned_portion_size == 0)
                        break;

                    __aligned_size -= __aligned_portion_size;
                    __advance_bytes(__stop_at, __aligned_portion_size);
                    __portion_begin = static_cast<const _ValueType*>(__first);

                    __current_values = datapar::load<_Simd_>(__first);

                    __current_values_max = __current_values;
                    __current_values_min = __current_values;

                    __current_indices_max = _IndexSimdType::zero();
                    __current_indices_min = _IndexSimdType::zero();
                }
                else {
                    break;
                }
            }
        }

        auto __first_current = static_cast<const _ValueType*>(__first);

        for (; __first_current != __last; ++__first_current ) {
            if (*__first_current < *__minmax.first)
                __minmax.first = __first_current;
            if (*__first_current > *__minmax.second)
                __minmax.second = __first_current;
        }

        return __minmax;
    }
};

template <class _Type_>
__raze_simd_algorithm_inline std::pair<const _Type_*, const _Type_*> __minmax_element_vectorized(
    const void* __first,
    const void* __last) noexcept
{
    return datapar::__simd_sized_dispatcher<__minmax_element_vectorized_internal, _Type_>()(
        __byte_length(__first, __last), &__minmax_element_scalar<_Type_>, __first, __last);
}

__RAZE_ALGORITHM_NAMESPACE_END