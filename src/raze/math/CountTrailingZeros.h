#pragma once 

#include <raze/Types.h>
#include <src/raze/RazeNamespace.h>
#include <src/raze/arch/CpuFeature.h>
#include <src/raze/arch/ProcessorDetection.h>
#include <src/raze/compatibility/Inline.h>
#include <src/raze/compatibility/MemoryMacros.h>
#include <src/raze/compatibility/SimdCompatibility.h>
#include <src/raze/traits/IntegralProperties.h>
#include <src/raze/vx/hw/x86/cast/SimdIntegralTypesCheck.h>
#include <concepts>

__RAZE_MATH_NAMESPACE_BEGIN

constexpr raze_always_inline i32 bit_hacks_ctz_u32(u32 v) noexcept {
    auto r = u32(32);
    v &= (u32(0) - v);

    if (v) --r;
    if (v & 0x0000FFFF) r -= 16;
    if (v & 0x00FF00FF) r -= 8;
    if (v & 0x0F0F0F0F) r -= 4;
    if (v & 0x33333333) r -= 2;
    if (v & 0x55555555) r -= 1;

    return static_cast<i32>(r);
}

template <std::unsigned_integral T>
constexpr raze_always_inline i32 bit_hacks_ctz(T v) noexcept {
    if constexpr (sizeof(T) == 8) {
        const auto low = static_cast<u32>(v);
        return low ? bit_hacks_ctz_u32(low)
            : 32 + bit_hacks_ctz_u32(static_cast<u32>(v >> 32));
    }
    else if constexpr (sizeof(T) == 4) {
        return bit_hacks_ctz_u32(static_cast<u32>(v));
    }
    else if constexpr (sizeof(T) == 2) {
        auto r = u32(16);
        v &= static_cast<u16>(u16(0) - v);

        if (v) --r;
        if (v & 0x000000FF) r -= 8;
        if (v & 0x00000F0F) r -= 4;
        if (v & 0x00003333) r -= 2;
        if (v & 0x00005555) r -= 1;

        return static_cast<i32>(r);
    }
    else if constexpr (sizeof(T) == 1) {
        auto r = u32(8);
        v &= static_cast<u8>(u8(0) - v);

        if (v) --r;
        if (v & 0x0000000F) r -= 4;
        if (v & 0x00000033) r -= 2;
        if (v & 0x00000055) r -= 1;

        return static_cast<i32>(r);
    }
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 ctz_bits_window(T u) noexcept {
    static_assert(Bits > 0 && Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);

    if consteval {
        if constexpr (Bits < 64) {
            const u64 masked = static_cast<u64>(u) & ((1ull << Bits) - 1ull);
            if constexpr (!Unsafe) {
                if (masked == 0) return static_cast<i32>(Bits);
            }
            return bit_hacks_ctz(masked);
        }
        else {
            const u64 val = static_cast<u64>(u);
            if constexpr (!Unsafe) {
                if (val == 0) return 64;
            }
            return bit_hacks_ctz(val);
        }
    }

#if defined(raze_cpp_msvc_only)
    if constexpr (Bits < 32) {
        const u32 val = [&]() raze_always_inline_lambda {
            if constexpr (Unsafe) return static_cast<u32>(u);
            else return static_cast<u32>(u) | (1u << Bits);
        }();
        if constexpr (vx::has_bmi1<ISA>) {
            return static_cast<i32>(_tzcnt_u32(val));
        }
        else {
            ulong index = 0;
            _BitScanForward(&index, val);
            return static_cast<i32>(index);
        }
    }
    else if constexpr (Bits == 32) {
        const u32 val = static_cast<u32>(u);
        if constexpr (vx::has_bmi1<ISA>) {
            return static_cast<i32>(_tzcnt_u32(val));
        }
        else {
            ulong index = 0;
            const auto r = _BitScanForward(&index, val);
            if constexpr (!Unsafe) { if (!r) return 32; }
            return static_cast<i32>(index);
        }
    }
    else {
        const u64 val = [&]() raze_always_inline_lambda {
            if constexpr (Bits == 64 || Unsafe) return static_cast<u64>(u);
            else return static_cast<u64>(u) | (1ull << Bits);
        }();
#  if defined(raze_processor_x86_64)
        if constexpr (vx::has_bmi1<ISA>) {
            return static_cast<i32>(_tzcnt_u64(val));
        }
        else {
            ulong index = 0;
            const auto r = _BitScanForward64(&index, val);
            if constexpr (Bits == 64 && !Unsafe) { if (!r) return 64; }
            return static_cast<i32>(index);
        }
#  else
        const u32 lo = static_cast<u32>(val);
        if (lo != 0) {
            if constexpr (vx::has_bmi1<ISA>) return static_cast<i32>(_tzcnt_u32(lo));
            ulong index = 0;
            _BitScanForward(&index, lo);
            return static_cast<i32>(index);
        }
        const u32 hi = static_cast<u32>(val >> 32);
        if constexpr (vx::has_bmi1<ISA>) return 32 + static_cast<i32>(_tzcnt_u32(hi));
        ulong index = 0;
        const auto r = _BitScanForward(&index, hi);
        if constexpr (Bits == 64 && !Unsafe) { if (!r) return 64; }
        return 32 + static_cast<i32>(index);
#  endif
    }
#else
    if constexpr (Bits < 32) {
        const u32 val = [&]() raze_always_inline_lambda {
            if constexpr (Unsafe) return static_cast<u32>(u);
            else return static_cast<u32>(u) | (1u << Bits);
        }();
        return static_cast<i32>(__builtin_ctz(val));
    }
    else if constexpr (Bits == 32) {
        const u32 val = static_cast<u32>(u);
        if constexpr (!Unsafe) { if (val == 0) return 32; }
        return static_cast<i32>(__builtin_ctz(val));
    }
    else if constexpr (Bits < 64) {
        const u64 val = [&]() raze_always_inline_lambda {
            if constexpr (Unsafe) return static_cast<u64>(u);
            else return static_cast<u64>(u) | (1ull << Bits);
        }();
        return static_cast<i32>(__builtin_ctzll(val));
    }
    else {
        const u64 val = static_cast<u64>(u);
        if constexpr (!Unsafe) { if (val == 0) return 64; }
        return static_cast<i32>(__builtin_ctzll(val));
    }
#endif
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe>
struct ctz_n_bits_implementation {
    template <std::unsigned_integral T>
    constexpr raze_always_inline i32 operator()(T v) const noexcept {
        return ctz_bits_window<ISA, Bits, Unsafe>(v);
    }
};

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 ctz_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);

    return ctz_n_bits_implementation<ISA, Bits, Unsafe>()(v);
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe>
struct ctz_not_n_bits_implementation {
    template <std::unsigned_integral T>
    constexpr raze_always_inline i32 operator()(T v) const noexcept {
        constexpr auto sent = Bits == raze_sizeof_in_bits(T) ? 0 : T(1ull << Bits);
        constexpr auto mask = T(sent - 1);

        return ctz_bits_window<ISA, Bits, Unsafe>(static_cast<T>(v ^ mask));
    }
};

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 ctz_not_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);
    
    return ctz_not_n_bits_implementation<ISA, Bits, Unsafe>()(v);
}

__RAZE_MATH_NAMESPACE_END