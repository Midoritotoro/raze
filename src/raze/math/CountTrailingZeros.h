#pragma once 

#include <raze/Types.h>
#include <src/raze/traits/IntegralProperties.h>

#include <raze/arch/ProcessorFeatures.h>
#include <raze/arch/ProcessorDetection.h>

#include <raze/compatibility/SimdCompatibility.h>

#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/math/SignBit.h>

__RAZE_MATH_NAMESPACE_BEGIN

#if defined (raze_processor_x86)
#  if defined(raze_cpp_clang) || defined(raze_cpp_gnu)

#    if !defined(raze_tzcnt_u32)
#      define raze_tzcnt_u32 __builtin_ia32_tzcnt_u32
#    endif // !defined(raze_tzcnt_u32)

#    if !defined(raze_tzcnt_u64)
#      define raze_tzcnt_u64 __builtin_ia32_tzcnt_u64
#    endif // !defined(raze_tzcnt_u64)

#  elif defined(raze_cpp_msvc)

#    if !defined(raze_tzcnt_u32)
#      define raze_tzcnt_u32 _tzcnt_u32
#    endif // !defined(raze_tzcnt_u32)

#    if !defined(raze_tzcnt_u64)
#      define raze_tzcnt_u64 _tzcnt_u64
#    endif // !defined(raze_tzcnt_u64)

#  endif // defined(raze_cpp_clang) || defined(raze_cpp_gnu) || defined(raze_cpp_msvc)
#endif // defined (raze_processor_x86)

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


#if defined (raze_processor_x86)

template <bool Unsafe, std::unsigned_integral T>
raze_always_inline i32 bsf_ctz(T v) noexcept {
    constexpr auto digits = std::numeric_limits<T>::digits;
    constexpr auto max = std::numeric_limits<T>::max();

    ulong index;

    if constexpr (digits == 64) {
        auto r = _BitScanForward64(&index, v);
        if constexpr (Unsafe) return index;
        else { if (!r) return digits; }
    }
    else {
        auto r = _BitScanForward(&index, v);
        if constexpr (Unsafe) return index;
        else { if (!r) return digits; }
    }

    return index;
}

template <std::unsigned_integral T>
raze_always_inline i32 tzcnt_ctz(T v) noexcept {
    constexpr auto digits = std::numeric_limits<T>::digits;
    if constexpr (digits == 64) return raze_tzcnt_u64(v);
    else return raze_tzcnt_u32(v);
}

#endif // defined(raze_processor_x86)

template <std::unsigned_integral T>
constexpr raze_always_inline i32 count_trailing_zero_bits_impl(T v) noexcept {
#if defined(raze_processor_x86)
    if not consteval {
        if (arch::ProcessorFeatures::AVX2())
            return tzcnt_ctz(v);
        else
            return bsf_ctz<false>(v);
    }
    else
#endif // defined(raze_processor_x86)
    {
        return bit_hacks_ctz(v);
    }
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe>
struct ctz_n_bits_implementation {
    template <std::unsigned_integral T>
    constexpr raze_always_inline i32 operator()(T v) const noexcept {
        constexpr auto mask_size = (Bits / 8) > 1 ? (Bits / 8) : 1;
        constexpr auto sent = Bits == raze_sizeof_in_bits(T) ? 0 : T(1ull << Bits);

        using UT = typename IntegerForSize<mask_size>::Unsigned;

        if constexpr (!vx::has_avx2<ISA>) {
            if constexpr (Bits != 32 && Bits != 64) return bsf_ctz<Unsafe>(static_cast<UT>(v | sent));
            else return bsf_ctz<Unsafe>(v);
        }
        else {
            if constexpr (Bits != 32 && Bits != 64) {
                constexpr auto nmax = (~std::numeric_limits<UT>::max()) >> (raze_sizeof_in_bits(UT) - Bits);
                return tzcnt_ctz(static_cast<u32>(i32(v) | nmax));
            }
            else return tzcnt_ctz(v);
        }
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
        constexpr auto mask_size = (Bits / 8) > 1 ? (Bits / 8) : 1;
        constexpr auto mask = T(sent - 1);
        constexpr auto sent = Bits == raze_sizeof_in_bits(T) ? 0 : T(1ull << Bits);

        using UT = typename IntegerForSize<mask_size>::Unsigned;

        if constexpr (!vx::has_avx2<ISA>) {
            if constexpr (Bits != 32 && Bits != 64) return bsf_ctz<Unsafe>(static_cast<UT>((v ^ mask) | sent));
            else return bsf_ctz<Unsafe>(v ^ mask);
        }
        else {
            if constexpr (Bits != 32 && Bits != 64) {
                constexpr auto nmax = (~std::numeric_limits<UT>::max()) >> (raze_sizeof_in_bits(UT) - Bits);
                return tzcnt_ctz(static_cast<u32>((i32(v) ^ mask) | nmax));
            }
            else return tzcnt_ctz(v ^ mask);
        }
    }
};

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 ctz_not_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);
    
    return ctz_not_n_bits_implementation<ISA, Bits, Unsafe>()(v);
}

__RAZE_MATH_NAMESPACE_END
