#pragma once 

#include <raze/Types.h>

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    sizetype _VerticalSize_,
    sizetype _HorizontalSize_,
    sizetype _Alignment_>
struct __shuffle_tables {
    alignas(_Alignment_) uint8 __shuffle[_VerticalSize_][_HorizontalSize_];
    alignas(_Alignment_) uint8 __size[_VerticalSize_];
};

template <
    sizetype _VerticalSize_,
    sizetype _HorizontalSize_,
    sizetype _Alignment_>
constexpr auto __make_shuffle_tables(
    const uint32 __multiplier,
    const uint32 __element_group_stride) noexcept
{
    auto __result = __shuffle_tables<_VerticalSize_, _HorizontalSize_, _Alignment_>();

    for (auto __vertical_index = uint32(0); __vertical_index != _VerticalSize_; ++__vertical_index) {
        auto __active_group_count = uint32(0);

        for (auto __horizontal_index = uint32(0); __horizontal_index != _HorizontalSize_ / __element_group_stride; ++__horizontal_index) {
            if ((__vertical_index & (1 << __horizontal_index)) == 0) {
                for (auto __element_offset = uint32(0); __element_offset != __element_group_stride; ++__element_offset)
                    __result.__shuffle[__vertical_index][__active_group_count * __element_group_stride + __element_offset] =
                        static_cast<uint8>(__horizontal_index * __element_group_stride + __element_offset);

                ++__active_group_count;
            }
        }

        __result.__size[__vertical_index] = static_cast<uint8>(__active_group_count * __multiplier);

        for (; __active_group_count != _HorizontalSize_ / __element_group_stride; ++__active_group_count)
            for (auto __element_offset = uint32(0); __element_offset != __element_group_stride; ++__element_offset)
                __result.__shuffle[__vertical_index][__active_group_count * __element_group_stride + __element_offset] =
                    static_cast<uint8>(__active_group_count * __element_group_stride + __element_offset);
    }

    return __result;
}

template <sizetype _Size_> 
constexpr auto __tables_sse = [] { 
    static_assert(_Size_ == 1 || _Size_ == 2 || _Size_ == 4 || _Size_ == 8, "Unsupported element size for __tables_sse");
    return __shuffle_tables<1, 1, 16>();
}();

template <>
constexpr auto __tables_sse<1>  = __make_shuffle_tables<256, 8, 16>(1, 1);

template <>
constexpr auto __tables_sse<2>  = __make_shuffle_tables<256, 16, 16>(2, 2);

template <>
constexpr auto __tables_sse<4>  = __make_shuffle_tables<16, 16, 16>(4, 4);

template <>
constexpr auto __tables_sse<8>  = __make_shuffle_tables<4, 16, 16>(8, 8);

template <sizetype _Size_>
constexpr auto __tables_avx = [] { 
    static_assert(_Size_ == 1 || _Size_ == 2 || _Size_ == 4 || _Size_ == 8, "Unsupported element size for __tables_avx");
    return __shuffle_tables<1, 1, 16>();
}();

template <>
constexpr auto __tables_avx<4> = __make_shuffle_tables<256, 8, 16>(4, 1);

template <>
constexpr auto __tables_avx<8> = __make_shuffle_tables<16, 8, 16>(8, 2);

__RAZE_DATAPAR_NAMESPACE_END
