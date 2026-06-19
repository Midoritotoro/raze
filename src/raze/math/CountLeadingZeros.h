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

template <bool _Unsafe_, std::unsigned_integral _IntegralType_>
raze_always_inline i32 __bsr_clz(_IntegralType_ __value) noexcept {
    constexpr auto __digits = std::numeric_limits<_IntegralType_>::digits;
    ulong __index;

    if constexpr (__digits == 64) {
        raze_maybe_unused_attribute auto __r = _BitScanReverse64(&__index, static_cast<u32>(__value));
        if constexpr (!_Unsafe_) { if (!__r) return __digits; }
    }
    else if constexpr (__digits <= 32) {
        raze_maybe_unused_attribute auto __r = _BitScanReverse(&__index, static_cast<u32>(__value));
        if constexpr (!_Unsafe_) { if (!__r) return __digits; }
    }

    return __digits - 1 - __index;
}

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

#if (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

template <std::unsigned_integral _IntegralType_>
raze_always_inline i32 __lzcnt_clz(_IntegralType_ __value) noexcept {
    constexpr auto __digits = std::numeric_limits<_IntegralType_>::digits;
    if constexpr (__digits == 64)  return __raze_lzcnt_u64(static_cast<u64>(__value));
    else if constexpr (__digits == 32) return __raze_lzcnt_u32(static_cast<u32>(__value)) - (32 - __digits);
    else return __raze_lzcnt_u16(static_cast<u16>(__value)) - (16 - __digits);
}

#endif // (defined(raze_processor_x86_32) || defined(raze_processor_x86_64))

template <std::unsigned_integral _IntegralType_>
constexpr raze_always_inline i32 __count_leading_zero_bits(_IntegralType_ __value) noexcept {
#if defined(raze_processor_x86) && !defined(raze_processor_arm)
    if (!type_traits::is_constant_evaluated()) {
        if (arch::ProcessorFeatures::AVX2())
            return __lzcnt_clz(__value);
        else
            return __bsr_clz<false>(__value);
    }
    else
#endif // defined(raze_processor_x86) && !defined(raze_processor_arm)
    {
        return __bit_hacks_clz(__value);
    }
}

template <arch::ISA _ISA_, sizetype _Bits_, bool _Unsafe_>
struct __clz_n_bits_implementation {
    template <std::unsigned_integral _IntegralType_>
    constexpr raze_always_inline i32 operator()(_IntegralType_ __value) const noexcept {
        constexpr auto __type_bits = raze_sizeof_in_bits(_IntegralType_);
        constexpr auto __offset = __type_bits - _Bits_;

        if constexpr (vx::__has_avx2_support_v<_ISA_>) return __lzcnt_clz(__value) - __offset;
        else return __bsr_clz<_Unsafe_>(__value) - __offset;
    }
};

template <arch::ISA _ISA_, sizetype _Bits_, bool _Unsafe_, std::unsigned_integral _IntegralType_>
constexpr raze_always_inline i32 __clz_n_bits(_IntegralType_ __value) noexcept {
    static_assert(_Bits_ <= 64);
    static_assert(raze_sizeof_in_bits(_IntegralType_) >= _Bits_);

    return __clz_n_bits_implementation<_ISA_, _Bits_, _Unsafe_>()(__value);
}

template <arch::ISA _ISA_, sizetype _Bits_, bool _Unsafe_>
struct __clz_not_n_bits_implementation {
    template <std::unsigned_integral _IntegralType_>
    constexpr raze_always_inline i32 operator()(_IntegralType_ __value) const noexcept {
        constexpr auto __type_bits = raze_sizeof_in_bits(_IntegralType_);
        constexpr auto __offset = __type_bits - _Bits_;
        
        constexpr auto __mask = _IntegralType_((_IntegralType_(1) << _Bits_) - 1);

        __value = _IntegralType_(~__value);
        if constexpr (_Bits_ < 8) __value &= __mask;

        if constexpr (vx::__has_avx2_support_v<_ISA_>) return __lzcnt_clz(__value) - __offset;
        else return __bsr_clz<_Unsafe_>(__value) - __offset;
    }
};

template <arch::ISA _ISA_, sizetype _Bits_, bool _Unsafe_, std::unsigned_integral _IntegralType_>
constexpr raze_always_inline i32 __clz_not_n_bits(_IntegralType_ __value) noexcept {
    static_assert(_Bits_ <= 64);
    static_assert(raze_sizeof_in_bits(_IntegralType_) >= _Bits_);

    return __clz_n_bits_implementation<_ISA_, _Bits_, _Unsafe_>()(__value);
}

__RAZE_MATH_NAMESPACE_END
