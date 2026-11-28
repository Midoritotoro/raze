#pragma once

#include <src/raze/vx/hw/x86/arithmetic/Abs.h>
#include <src/raze/vx/hw/x86/compare/Less.h>
#include <src/raze/vx/hw/x86/compare/NotEqual.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>
#include <raze/math/BitCast.h>
#include <limits>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline auto is_subnormal_(V x) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm_fpclass_pd_mask(as<__m128d>(x), 0x20);
        else if constexpr (ps<T>) return _mm_fpclass_ps_mask(as<__m128>(x), 0x20);
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm256_fpclass_pd_mask(as<__m256d>(x), 0x20);
        else if constexpr (ps<T>) return _mm256_fpclass_ps_mask(as<__m256>(x), 0x20);
    }
    else if constexpr (sizeof(V) == 64 && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm512_fpclass_pd_mask(as<__m512d>(x), 0x20);
        else if constexpr (ps<T>) return _mm512_fpclass_ps_mask(as<__m512>(x), 0x20);
    }
    else if constexpr (arithmetic_type<V>) {
        if constexpr (pd<T>) {
            const u64 a = math::bit_cast<u64>(x) & 0x7FFFFFFFFFFFFFFFull;
            return a != 0ull && a < 0x0010000000000000ull;
        }
        else {
            const u32 a = math::bit_cast<u32>(x) & 0x7FFFFFFFu;
            return a != 0u && a < 0x00800000u;
        }
    }
    else {
        const auto ax = abs_<ISA, T>(x);
        return mask_and_<ISA, T>(
            not_equal_<ISA, T>(ax, zero_<ISA, V, T>()),
            less_<ISA, T>(ax, broadcast_<ISA, V>(std::numeric_limits<T>::min())));
    }
}

template <arch::ISA ISA, std::floating_point T, intrin_or_arithmetic_type V, raw_mask_type Mask>
raze_nodiscard raze_always_inline auto is_subnormal_(V x, Mask mask) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm_mask_fpclass_pd_mask(mask, as<__m128d>(x), 0x20);
        else if constexpr (ps<T>) return _mm_mask_fpclass_ps_mask(mask, as<__m128>(x), 0x20);
    }
    else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA> && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm256_mask_fpclass_pd_mask(mask, as<__m256d>(x), 0x20);
        else if constexpr (ps<T>) return _mm256_mask_fpclass_ps_mask(mask, as<__m256>(x), 0x20);
    }
    else if constexpr (sizeof(V) == 64 && has_avx512dq<ISA>) {
        if constexpr (pd<T>) return _mm512_mask_fpclass_pd_mask(mask, as<__m512d>(x), 0x20);
        else if constexpr (ps<T>) return _mm512_mask_fpclass_ps_mask(mask, as<__m512>(x), 0x20);
    }

    return mask_and_<ISA, T>(is_subnormal_<ISA, T>(x), mask);
}

__RAZE_VX_NAMESPACE_END

