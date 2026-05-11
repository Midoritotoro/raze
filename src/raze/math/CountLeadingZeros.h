#pragma once 

#include <src/raze/math/PopulationCount.h>

__RAZE_MATH_NAMESPACE_BEGIN

#if defined (raze_processor_x86)
#  if defined(raze_cpp_clang) || defined(raze_cpp_gnu)

#    if !defined(__raze_lzcnt_u16)
#      define __raze_lzcnt_u16 __builtin_ia32_lzcnt_u16
#    endif // !defined(__raze_lzcnt_u16)

#    if !defined(__raze_lzcnt_u32)
#      define __raze_lzcnt_u32 __builtin_ia32_lzcnt_u32
#    endif // !defined(__raze_lzcnt_u32)

#    if !defined(__raze_lzcnt_u64)
#      define __raze_lzcnt_u64 __builtin_ia32_lzcnt_u64 
#    endif // !defined(__raze_lzcnt_u64)

#  elif defined(raze_cpp_msvc)

#    if !defined(__raze_lzcnt_u16)
#      define __raze_lzcnt_u16 __lzcnt16
#    endif // !defined(__raze_lzcnt_u16)

#    if !defined(__raze_lzcnt_u32)
#      define __raze_lzcnt_u32 __lzcnt
#    endif // !defined(__raze_lzcnt_u32)

#    if !defined(__raze_lzcnt_u64)
#      define __raze_lzcnt_u64 __lzcnt64
#    endif // !defined(__raze_lzcnt_u64)

#  endif // defined(raze_cpp_clang) || defined(raze_cpp_gnu) || defined(raze_cpp_msvc)
#endif // defined (raze_processor_x86)

template <std::unsigned_integral _IntegralType_>
constexpr raze_always_inline i32 __bit_hacks_clz(_IntegralType_ __value) noexcept {
	if constexpr (sizeof(_IntegralType_) == 8) {
        __value = __value | (__value >> 1);
        __value = __value | (__value >> 2);
        __value = __value | (__value >> 4);

        __value = __value | (__value >> 8);
        __value = __value | (__value >> 16);
        __value = __value | (__value >> 32);

        return __bit_hacks_population_count(static_cast<_IntegralType_>(~__value));
	}
    else if constexpr (sizeof(_IntegralType_) == 4) {
        __value = __value | (__value >> 1);
        __value = __value | (__value >> 2);

        __value = __value | (__value >> 4);
        __value = __value | (__value >> 8);

        __value = __value | (__value >> 16);
        
        return __bit_hacks_population_count(static_cast<_IntegralType_>(~__value));
    }
    else if constexpr (sizeof(_IntegralType_) == 2) {
        __value = __value | (__value >> 1);
        __value = __value | (__value >> 2);

        __value = __value | (__value >> 4);
        __value = __value | (__value >> 8);

        return __bit_hacks_population_count(static_cast<_IntegralType_>(~__value));
    }
    else if constexpr (sizeof(_IntegralType_) == 1) {
        __value = __value | (__value >> 1);

        __value = __value | (__value >> 2);
        __value = __value | (__value >> 4);

        return __bit_hacks_population_count(static_cast<_IntegralType_>(~__value));
    }
}

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

template <std::unsigned_integral _IntegralType_>
raze_always_inline i32 __bsr_clz(_IntegralType_ __value) noexcept {
    constexpr auto __digits = std::numeric_limits<_IntegralType_>::digits;
    auto __index = ulong(0);

    if constexpr (__digits == 64) {
        if (!_BitScanReverse64(&__index, __value))
            return __digits;
    }
    else if constexpr (__digits <= 32) {
        if (!_BitScanReverse(&__index, __value))
            return __digits;
    }

    return static_cast<int>(__digits - 1 - __index);
}

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

template <std::unsigned_integral _IntegralType_>
raze_always_inline i32 __lzcnt_clz(_IntegralType_ __value) noexcept {
    constexpr auto __digits = std::numeric_limits<_IntegralType_>::digits;

    if      constexpr (__digits == 64)
        return static_cast<int>(__raze_lzcnt_u64(static_cast<u64>(__value)));
    else if constexpr (__digits == 32)
        return static_cast<int>(__raze_lzcnt_u32(static_cast<u32>(__value)));
    else
        return static_cast<int>(__raze_lzcnt_u16(static_cast<u16>(__value))) - (16 - __digits);
}

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

template <std::unsigned_integral _IntegralType_>
constexpr raze_always_inline i32 __count_leading_zero_bits(_IntegralType_ __value) noexcept {
#if defined(raze_processor_x86) && !defined(raze_processor_arm)
    if (!type_traits::is_constant_evaluated()) {
        if (arch::ProcessorFeatures::AVX2())
            return __lzcnt_clz(__value);
        else
            return __bsr_clz(__value);
    }
    else
#endif // defined(raze_processor_x86) && !defined(raze_processor_arm)
    {
        return __bit_hacks_clz(__value);
    }
}

template <sizetype _Bits_>
struct __clz_n_bits_implementation {
    template <std::unsigned_integral _IntegralType_>
    constexpr raze_always_inline i32 operator()(_IntegralType_ __value) const noexcept {
        constexpr auto __max_for_n_bits = _IntegralType_(((_IntegralType_(1) << _Bits_) - 1));
        constexpr auto __mask_size = (_Bits_ / 8) > 1 ? (_Bits_ / 8) : 1;

        using _UintForBits = typename IntegerForSize<__mask_size>::Unsigned;

        return __count_leading_zero_bits(static_cast<_UintForBits>(__value & __max_for_n_bits));
    }
};

template <>
struct __clz_n_bits_implementation<2> {
    template <std::unsigned_integral _IntegralType_>
    constexpr raze_always_inline i32 operator()(_IntegralType_ __value) const noexcept {
        __value &= 0x03;
        __value = __value | (__value >> 1);
        return __popcnt_n_bits<2>(static_cast<_IntegralType_>(~__value));
    }
};

template <>
struct __clz_n_bits_implementation<4> {
    template <std::unsigned_integral _IntegralType_>
    constexpr raze_always_inline i32 operator()(_IntegralType_ __value) const noexcept {
        __value &= 0xF;

        __value = __value | (__value >> 1);
        __value = __value | (__value >> 2);

        return __popcnt_n_bits<4>(static_cast<_IntegralType_>(~__value));
    }
};

template <
    sizetype _Bits_, 
    std::unsigned_integral _IntegralType_>
constexpr raze_always_inline i32 __clz_n_bits(_IntegralType_ __value) noexcept {
    static_assert(_Bits_ <= 64);
    static_assert(raze_sizeof_in_bits(_IntegralType_) >= _Bits_);

    return __clz_n_bits_implementation<_Bits_>()(__value);
}

__RAZE_MATH_NAMESPACE_END
