#pragma once

#include <src/raze/vx/hw/x86/arithmetic/Div.h>
#include <src/raze/vx/hw/x86/arithmetic/Mul.h>
#include <src/raze/vx/hw/x86/arithmetic/Sub.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, std::integral T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V rem_fallback_(V x, V y) noexcept {
    if constexpr (arithmetic_type<V>) {
        return static_cast<V>(x % y);
    }
    else {
        constexpr sizetype size = sizeof(V) / sizeof(T);

        alignas(sizeof(V)) T dest[size];
        alignas(sizeof(V)) T left[size];
        alignas(sizeof(V)) T right[size];

        store_(left, x, aligned_policy{});
        store_(right, y, aligned_policy{});

        for (sizetype i = 0; i < size; ++i)
            dest[i] = static_cast<T>(left[i] % right[i]);

        return load_<ISA, V>(dest, aligned_policy{});
    }
}

template <arch::ISA ISA, std::integral T, intrin_type V>
raze_nodiscard raze_always_inline V rem_fallback_(V x, T y) noexcept {
    constexpr sizetype size = sizeof(V) / sizeof(T);

    alignas(sizeof(V)) T dest[size];
    alignas(sizeof(V)) T left[size];

    store_(left, x, aligned_policy{});

    for (sizetype i = 0; i < size; ++i)
        dest[i] = static_cast<T>(left[i] % y);

    return load_<ISA, V>(dest, aligned_policy{});
}

template <arch::ISA ISA, std::integral T, intrin_or_arithmetic_type V>
raze_nodiscard raze_always_inline V rem_(V x, V y) noexcept {
    if constexpr (sizeof(V) == 16 && has_avx<ISA> && epi32<T>) {
        const auto q = div_<ISA, T>(x, y);
        return sub_<ISA, T>(x, mul_<ISA, T>(q, y));
    }

    return rem_fallback_<ISA, T>(x, y);
}

template <arch::ISA ISA, std::integral T, intrin_type V>
raze_nodiscard raze_always_inline V rem_(V x, T y) noexcept {
    if constexpr (sizeof(V) == 16) {
        if constexpr (epi64<T> || epu64<T>) {
#if !defined(raze_processor_x86_32)
            const auto left_0 = static_cast<T>(_mm_cvtsi128_si64(as<__m128i>(x)));
            const auto left_1 = static_cast<T>(_mm_cvtsi128_si64(as<__m128i>(_mm_movehl_ps(
                as<__m128>(x), as<__m128>(x)))));

            return as<V>(_mm_set_epi64x(static_cast<i64>(left_1 % y), static_cast<i64>(left_0 % y)));
#else
            return rem_fallback_<ISA, T>(x, y);
#endif
        }
        else {
            const auto q = div_<ISA, T>(x, y);
            return sub_<ISA, T>(x, mul_<ISA, T>(q, broadcast_<ISA, V>(y)));
        }
    }
    else if constexpr (sizeof(V) == 32 && has_avx2<ISA> && sizeof(T) <= 4) {
        const auto q = div_<ISA, T>(x, y);
        return sub_<ISA, T>(x, mul_<ISA, T>(q, broadcast_<ISA, V>(y)));
    }
    else if constexpr (sizeof(V) == 64 && sizeof(T) == 4) {
        const auto q = div_<ISA, T>(x, y);
        return sub_<ISA, T>(x, mul_<ISA, T>(q, broadcast_<ISA, V>(y)));
    }

    return rem_fallback_<ISA, T>(x, y);
}

template <arch::ISA ISA, std::integral T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V rem_(V x, V y, M mask) noexcept {
    if constexpr (arithmetic_type<V>) {
        return mask ? static_cast<V>(x % y) : static_cast<V>(0);
    }
    else {
        const auto safe_y = select_<ISA, T>(y, broadcast_<ISA, V>(T(1)), mask);
        return select_<ISA, T>(rem_<ISA, T>(x, safe_y), mask);
    }
}

template <arch::ISA ISA, std::integral T, intrin_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V rem_(V x, T y, M mask) noexcept {
    return select_<ISA, T>(rem_<ISA, T>(x, y), mask);
}

template <arch::ISA ISA, std::integral T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V rem_(V x, V y, M mask, V src) noexcept {
    if constexpr (arithmetic_type<V>) {
        return mask ? static_cast<V>(x % y) : src;
    }
    else {
        const auto safe_y = select_<ISA, T>(y, broadcast_<ISA, V>(T(1)), mask);
        return select_<ISA, T>(rem_<ISA, T>(x, safe_y), src, mask);
    }
}

template <arch::ISA ISA, std::integral T, intrin_type V, raw_mask_type M>
raze_nodiscard raze_always_inline V rem_(V x, T y, M mask, V src) noexcept {
    return select_<ISA, T>(rem_<ISA, T>(x, y), src, mask);
}

__RAZE_VX_NAMESPACE_END

