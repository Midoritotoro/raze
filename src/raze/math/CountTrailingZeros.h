#pragma once 

#include <raze/Types.h>
#include <src/raze/traits/IntegralProperties.h>

#include <raze/arch/ProcessorFeatures.h>
#include <raze/arch/ProcessorDetection.h>

#include <raze/compatibility/SimdCompatibility.h>

#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/math/SignBit.h>

__RAZE_MATH_NAMESPACE_BEGIN

constexpr raze_always_inline i32 bit_hacks_ctz_u32(u32 v) noexcept {
    auto r = u32(32);
    v &= -signed(v);

    if (v) --r;
    if (v & 0x0000FFFF) r -= 16;
    if (v & 0x00FF00FF) r -= 8;
    if (v & 0x0F0F0F0F) r -= 4;
    if (v & 0x33333333) r -= 2;
    if (v & 0x55555555) r -= 1;

    return r;
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
        v &= u16(-signed(v));

        if (v) --r;
        if (v & 0x000000FF) r -= 8;
        if (v & 0x00000F0F) r -= 4;
        if (v & 0x00003333) r -= 2;
        if (v & 0x00005555) r -= 1;

        return r;
    }
    else if constexpr (sizeof(T) == 1) {
        auto r = u32(8);
        v &= u8(-signed(v));

        if (v) --r;
        if (v & 0x0000000F) r -= 4;
        if (v & 0x00000033) r -= 2;
        if (v & 0x00000055) r -= 1;

        return r;
    }
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 ctz_bits_window(T u) noexcept {
    constexpr auto digits    = std::numeric_limits<T>::digits;
    constexpr auto mask_size = (Bits / 8) > 1 ? (Bits / 8) : 1;
    constexpr auto sent      = Bits == raze_sizeof_in_bits(T) ? 0 : T(1ull << Bits);
    using UT = typename IntegerForSize<mask_size>::Unsigned;

#if defined(raze_cpp_msvc_only)
    if constexpr (!vx::has_bmi1<ISA>) {
        ulong index;
        if constexpr (Bits != 32 && Bits != 64) {
            auto r = _BitScanForward(&index, static_cast<u32>(static_cast<UT>(u | sent)));
            if constexpr (!Unsafe) { if (!r) return Bits; }
        } 
        else if constexpr (Bits == 64) {
            auto r = _BitScanForward64(&index, static_cast<u64>(u));
            if constexpr (!Unsafe) { if (!r) return digits; }
        } 
        else {
            auto r = _BitScanForward(&index, static_cast<u32>(u));
            if constexpr (!Unsafe) { if (!r) return digits; }
        }
        return static_cast<i32>(index);
    } else {
        if constexpr (Bits != 32 && Bits != 64) {
            constexpr auto nmax = (~std::numeric_limits<UT>::max()) >> (raze_sizeof_in_bits(UT) - Bits);
            return static_cast<i32>(_tzcnt_u32(static_cast<u32>(i32(u) | nmax)));
        } 
        else if constexpr (Bits == 64) return static_cast<i32>(_tzcnt_u64(static_cast<u64>(u)));
        else return static_cast<i32>(_tzcnt_u32(static_cast<u32>(u)));
    }
#else
    if constexpr (!vx::has_bmi1<ISA>) {
        if constexpr (Bits != 32 && Bits != 64) {
            const UT uu = static_cast<UT>(u | sent);
            if constexpr (!Unsafe) { if (uu == 0) return Bits; }
            return static_cast<i32>(__builtin_ctz(static_cast<unsigned>(uu)));
        }
        else {
            if constexpr (!Unsafe) { if (u == 0) return digits; }
            if constexpr (Bits == 64) return static_cast<i32>(__builtin_ctzll(static_cast<unsigned long long>(u)));
            else return static_cast<i32>(__builtin_ctz(static_cast<unsigned>(u)));
        }
    } else {
        if constexpr (Bits != 32 && Bits != 64) {
            constexpr auto nmax = (~std::numeric_limits<UT>::max()) >> (raze_sizeof_in_bits(UT) - Bits);
            return static_cast<i32>(__builtin_ctz(static_cast<unsigned>(static_cast<u32>(i32(u) | nmax))));
        } 
        else if constexpr (Bits == 64) return u ? static_cast<i32>(__builtin_ctzll(static_cast<unsigned long long>(u))) : digits;
        else return u ? static_cast<i32>(__builtin_ctz(static_cast<unsigned>(u))) : digits;
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