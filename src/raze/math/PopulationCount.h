#pragma once

#include <raze/Types.h>
#include <src/raze/type_traits/IntegralProperties.h>

#include <raze/arch/ProcessorFeatures.h>
#include <raze/arch/ProcessorDetection.h>

#include <raze/compatibility/SimdCompatibility.h>
#include <src/raze/type_traits/IsConstantEvaluated.h>

#include <include/raze/math/IntegralTypesConversions.h>

raze_disable_warning_msvc(4293)

__RAZE_MATH_NAMESPACE_BEGIN

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))

#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)

#    if !defined(__raze_popcnt_u32)
#      define __raze_popcnt_u32 __builtin_popcount
#    endif // !defined(__raze_popcnt_u32)

#    if !defined(__raze_popcnt_u64)
#      define __raze_popcnt_u64 __builtin_popcountll
#    endif // !defined(__raze_popcnt_u64)

#  elif defined(raze_cpp_msvc) 

#    if !defined(__raze_popcnt_u32)
#      define __raze_popcnt_u32 __popcnt
#    endif // !defined(__raze_popcnt_u32)

#    if !defined(__raze_popcnt_u64)
#      define __raze_popcnt_u64 __popcnt64
#    endif // !defined(__raze_popcnt_u64)

#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))


template <typename _IntegralType_>
constexpr raze_always_inline int __bit_hacks_population_count(_IntegralType_ __value) noexcept {
    if      constexpr (sizeof(_IntegralType_) == 8) {
        return (((__value) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 12) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 24) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 36) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 48) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 60) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f;
    }
    else if constexpr (sizeof(_IntegralType_) == 4) {
        return (((__value) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 12) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
        (((__value >> 24) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f;
    }
    else if constexpr (sizeof(_IntegralType_) == 2) {
        return (((__value) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f +
            (((__value >> 12) & 0xfff) * static_cast<uint64>(0x1001001001001)
                & static_cast<uint64>(0x84210842108421)) % 0x1f;
    }
    else if constexpr (sizeof(_IntegralType_) == 1) {
        return (((__value) & 0xfff) * static_cast<uint64>(0x1001001001001)
            & static_cast<uint64>(0x84210842108421)) % 0x1f;
    }
}

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))

template <typename _IntegralType_>
raze_always_inline int __popcnt_population_count(_IntegralType_ __value) noexcept {
    constexpr auto __digits = std::numeric_limits<_IntegralType_>::digits;

    if      constexpr (__digits == 64)
        return static_cast<int>(__raze_popcnt_u64(static_cast<uint64>(__value)));
    else if constexpr (__digits == 32)
        return static_cast<int>(__raze_popcnt_u32(static_cast<uint32>(__value)));
    else if constexpr (__digits <= 16)
#if defined(raze_cpp_msvc)
        return static_cast<int>(__popcnt16(static_cast<uint16>(__value)));
#elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
        return __bit_hacks_population_count(__value);
#endif // defined(raze_cpp_msvc) // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
}

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))

template <typename _IntegralType_>
constexpr raze_always_inline int __population_count(_IntegralType_ __value) noexcept {
    static_assert(std::is_unsigned_v<_IntegralType_>);

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))
    if (!type_traits::is_constant_evaluated()) {
        if (arch::ProcessorFeatures::POPCNT())
            return __popcnt_population_count(__value);
    }
    else
#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64) || defined(raze_processor_arm_64))
    {
        return __bit_hacks_population_count(__value);
    }
}

template <
    sizetype _Bits_,
    typename _IntegralType_>
constexpr raze_always_inline int __popcnt_n_bits(_IntegralType_ __value) noexcept {
    static_assert(_Bits_ <= 64);
    static_assert(_Bits_ <= (sizeof(_IntegralType_) * 8));

    constexpr auto __max_for_n_bits = _IntegralType_(((_IntegralType_(1) << _Bits_) - 1));
    constexpr auto __mask_size = (_Bits_ / 8) > 1 ? (_Bits_ / 8) : 1;

    using _UintForBits = typename IntegerForSize<__mask_size>::Unsigned;

    if constexpr (_Bits_ >= 8)
        return __population_count(static_cast<_UintForBits>(__value & __max_for_n_bits));
    else 
        return __bit_hacks_population_count(static_cast<_UintForBits>(__value & __max_for_n_bits));
}

__RAZE_MATH_NAMESPACE_END
