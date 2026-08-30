#pragma once 

#include <raze/Types.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype VerticalSize, sizetype HorizontalSize, sizetype Alignment>
struct compress_tables {
    alignas(Alignment) u8 size[VerticalSize];
    alignas(Alignment) u8 shuffle[VerticalSize][HorizontalSize];
};

template <sizetype VerticalSize, sizetype HorizontalSize, sizetype Alignment>
constexpr auto make_compress_tables(u32 multiplier, u32 element_group_stride) noexcept {
    auto result = compress_tables<VerticalSize, HorizontalSize, Alignment>();

    for (auto vertical_index = u32(0); vertical_index != VerticalSize; ++vertical_index) {
        auto active_group_count = u32(0);

        for (auto horizontal_index = u32(0); horizontal_index != HorizontalSize / element_group_stride; ++horizontal_index) {
            if ((vertical_index & (1 << horizontal_index)) == 0) {
                for (auto element_offset = u32(0); element_offset != element_group_stride; ++element_offset)
                    result.shuffle[vertical_index][active_group_count * element_group_stride + element_offset] =
                        static_cast<u8>(horizontal_index * element_group_stride + element_offset);

                ++active_group_count;
            }
        }

        result.size[vertical_index] = static_cast<u8>(active_group_count * multiplier);

        for (; active_group_count != HorizontalSize / element_group_stride; ++active_group_count)
            for (auto element_offset = u32(0); element_offset != element_group_stride; ++element_offset)
                result.shuffle[vertical_index][active_group_count * element_group_stride + element_offset] =
                    static_cast<u8>(active_group_count * element_group_stride + element_offset);
    }

    return result;
}

template <sizetype N> 
constexpr auto tables_sse = [] { 
    static_assert(N == 1 || N == 2 || N == 4 || N == 8, "Unsupported element size for tables_sse");
    return compress_tables<1, 1, 16>();
}();

template <> constexpr auto tables_sse<1>  = make_compress_tables<256, 8, 16>(1, 1);
template <> constexpr auto tables_sse<2>  = make_compress_tables<256, 16, 16>(2, 2);
template <> constexpr auto tables_sse<4>  = make_compress_tables<16, 16, 16>(4, 4);
template <> constexpr auto tables_sse<8>  = make_compress_tables<4, 16, 16>(8, 8);

template <sizetype N>
constexpr auto tables_avx = [] { 
    static_assert(N == 1 || N == 2 || N == 4 || N == 8, "Unsupported element size for tables_avx");
    return compress_tables<1, 1, 16>();
}();

template <> constexpr auto tables_avx<4> = make_compress_tables<256, 8, 16>(4, 1);
template <> constexpr auto tables_avx<8> = make_compress_tables<16, 8, 16>(8, 2);

__RAZE_VX_NAMESPACE_END
