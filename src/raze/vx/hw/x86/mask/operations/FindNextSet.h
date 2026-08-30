#pragma once 

#include <src/raze/math/CountTrailingZeros.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, bool Unsafe, raw_mask_type M>
raze_always_inline i32 find_next_set_(M x, u32 shift) noexcept {
    if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
        return shift == 0 ? !x : 1;
    }
    else {
        auto mask = to_mask_<ISA, T>(x);
        using Mask = decltype(mask);

        Mask mask_after = shift == 0 ? mask : Mask(mask & Mask(Mask(~0) << shift));
        return math::ctz_n_bits<ISA, N, Unsafe>(mask_after);
    }
}

__RAZE_VX_NAMESPACE_END
