#pragma once

#include <raze/Types.h>
#include <src/raze/traits/IntegralProperties.h>

#include <raze/arch/ProcessorFeatures.h>
#include <raze/arch/ProcessorDetection.h>

#include <raze/compatibility/SimdCompatibility.h>
#include <src/raze/math/IntegralTypesConversions.h>

raze_disable_warning_msvc(4293)

__RAZE_MATH_NAMESPACE_BEGIN

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))

#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)

#    if !defined(raze_popcnt_u32)
#      define raze_popcnt_u32 __builtin_popcount
#    endif // !defined(raze_popcnt_u32)

#    if !defined(raze_popcnt_u64)
#      define raze_popcnt_u64 __builtin_popcountll
#    endif // !defined(raze_popcnt_u64)

#  elif defined(raze_cpp_msvc) 

#    if !defined(raze_popcnt_u32)
#      define raze_popcnt_u32 __popcnt
#    endif // !defined(raze_popcnt_u32)

#    if !defined(raze_popcnt_u64)
#      define raze_popcnt_u64 __popcnt64
#    endif // !defined(raze_popcnt_u64)

#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))


template <std::unsigned_integral T>
constexpr raze_always_inline i32 bit_hacks_population_count(T v) noexcept {
    if constexpr (sizeof(T) == 8) {
        return (((v) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 12) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 24) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 36) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 48) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 60) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f;
    }
    else if constexpr (sizeof(T) == 4) {
        return (((v) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 12) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
        (((v >> 24) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f;
    }
    else if constexpr (sizeof(T) == 2) {
        return (((v) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f +
            (((v >> 12) & 0xfff) * static_cast<u64>(0x1001001001001)
                & static_cast<u64>(0x84210842108421)) % 0x1f;
    }
    else if constexpr (sizeof(T) == 1) {
        return (((v) & 0xfff) * static_cast<u64>(0x1001001001001)
            & static_cast<u64>(0x84210842108421)) % 0x1f;
    }
}

#if defined(raze_processor_x86)

template <std::unsigned_integral T>
raze_always_inline i32 popcnt_population_count(T v) noexcept {
    constexpr auto digits = std::numeric_limits<T>::digits;

    if constexpr (digits == 64) return static_cast<int>(raze_popcnt_u64(static_cast<u64>(v)));
    else return static_cast<int>(raze_popcnt_u32(static_cast<u32>(v)));
}

#endif // defined(raze_processor_x86)

template <std::unsigned_integral T>
constexpr raze_always_inline i32 population_count_impl(T v) noexcept {
#if defined(raze_processor_x86)
    if !consteval {
        if (arch::ProcessorFeatures::POPCNT())
            return popcnt_population_count(v);
    }
#endif // defined(raze_processor_x86)
    return bit_hacks_population_count(v);
}

template <sizetype Bits, std::unsigned_integral T>
constexpr raze_always_inline i32 popcnt_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(Bits <= raze_sizeof_in_bits(T));

    constexpr auto max_for_n_bits = (Bits == raze_sizeof_in_bits(T))
        ? math::max_limit<T>()
        : T(((T(1) << Bits) - 1));
    constexpr auto mask_size = (Bits / 8) > 1 ? (Bits / 8) : 1;

    using UT = typename IntegerForSize<mask_size>::Unsigned;
    return population_count_impl(static_cast<UT>(v & max_for_n_bits));
}

template <sizetype Bits, std::unsigned_integral T>
constexpr raze_always_inline i32 native_popcnt_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(Bits <= raze_sizeof_in_bits(T));

    constexpr auto max_for_n_bits = (Bits == raze_sizeof_in_bits(T))
        ? math::max_limit<T>()
        : T(((T(1) << Bits) - 1));
    constexpr auto mask_size = (Bits / 8) > 1 ? (Bits / 8) : 1;

    using UT = typename IntegerForSize<mask_size>::Unsigned;
    return popcnt_population_count(static_cast<UT>(v & max_for_n_bits));
}

__RAZE_MATH_NAMESPACE_END
