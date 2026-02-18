#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>

#include <array>


__RAZE_DATAPAR_NAMESPACE_BEGIN

static constexpr auto __make_8bit_mask_expand_table() noexcept {
    std::array<uint64, 256> __result;

    for (auto __index = 0; __index < 256; ++__index) {
        auto __temp = uint64(0);

        for (auto __index2 = 0; __index2 < 8; ++__index2)
            if (__index & (1 << __index2))
                __temp |= (uint64(0xFF) << (__index2 * 8));

        __result[__index] = __temp;
    }

    return __result;
}

static constexpr auto __make_16bit_mask_expand_table() noexcept {
    std::array<uint32, 256> __result;

    for (auto __index = 0; __index < 256; ++__index) {
        auto __temp = uint32(0);

        for (auto __index2 = 0; __index2 < 8; ++__index2)
            if (__index & (1 << __index2))
                __temp |= (0xFu << (__index2 * 4));

        __result[__index] = __temp;
    }

    return __result;
}

static inline constexpr auto __mask_8bit_expand_table_avx512bw = __make_8bit_mask_expand_table();
static inline constexpr auto __mask_16bit_expand_table_avx512bw = __make_16bit_mask_expand_table();

template <typename _Type_>
raze_always_inline auto __expand_mask_bits_zmm(_Type_ __mask) noexcept {
    if constexpr (sizeof(_Type_) == 1) {
        return __mask_8bit_expand_table_avx512bw[__mask];
    }
    else if constexpr (sizeof(_Type_) == 2) {
        const auto __low  = uint8(__mask & 0xFF);
        const auto __high = uint8(__mask >> 8);

        return ((static_cast<uint64>(__mask_16bit_expand_table_avx512bw[__high]) << 32)) | __mask_16bit_expand_table_avx512bw[__low];
    }
    else {
        return __mask;
    }

}

__RAZE_DATAPAR_NAMESPACE_END