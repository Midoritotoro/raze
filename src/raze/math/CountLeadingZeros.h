#pragma once 

#include <src/raze/math/PopulationCount.h>

__RAZE_MATH_NAMESPACE_BEGIN

#if defined (raze_processor_x86)
#  if defined(raze_cpp_clang) || defined(raze_cpp_gnu)

#    if !defined(raze_lzcnt_u16)
#      define raze_lzcnt_u16 __builtin_ia32_lzcnt_u16
#    endif // !defined(raze_lzcnt_u16)

#    if !defined(raze_lzcnt_u32)
#      define raze_lzcnt_u32 __builtin_ia32_lzcnt_u32
#    endif // !defined(raze_lzcnt_u32)

#    if !defined(raze_lzcnt_u64)
#      define raze_lzcnt_u64 __builtin_ia32_lzcnt_u64 
#    endif // !defined(raze_lzcnt_u64)

#  elif defined(raze_cpp_msvc)

#    if !defined(raze_lzcnt_u16)
#      define raze_lzcnt_u16 __lzcnt16
#    endif // !defined(raze_lzcnt_u16)

#    if !defined(raze_lzcnt_u32)
#      define raze_lzcnt_u32 __lzcnt
#    endif // !defined(raze_lzcnt_u32)

#    if !defined(raze_lzcnt_u64)
#      define raze_lzcnt_u64 __lzcnt64
#    endif // !defined(raze_lzcnt_u64)

#  endif // defined(raze_cpp_clang) || defined(raze_cpp_gnu) || defined(raze_cpp_msvc)
#endif // defined (raze_processor_x86)

template <std::unsigned_integral T>
constexpr raze_always_inline i32 bit_hacks_clz(T v) noexcept {
	if constexpr (sizeof(T) == 8) {
        v = v | (v >> 1);
        v = v | (v >> 2);
        v = v | (v >> 4);

        v = v | (v >> 8);
        v = v | (v >> 16);
        v = v | (v >> 32);

        return bit_hacks_population_count(static_cast<T>(~v));
	}
    else if constexpr (sizeof(T) == 4) {
        v = v | (v >> 1);
        v = v | (v >> 2);

        v = v | (v >> 4);
        v = v | (v >> 8);

        v = v | (v >> 16);
        
        return bit_hacks_population_count(static_cast<T>(~v));
    }
    else if constexpr (sizeof(T) == 2) {
        v = v | (v >> 1);
        v = v | (v >> 2);

        v = v | (v >> 4);
        v = v | (v >> 8);

        return bit_hacks_population_count(static_cast<T>(~v));
    }
    else if constexpr (sizeof(T) == 1) {
        v = v | (v >> 1);

        v = v | (v >> 2);
        v = v | (v >> 4);

        return bit_hacks_population_count(static_cast<T>(~v));
    }
}

#if defined(raze_processor_x86)

template <bool Unsafe, std::unsigned_integral T>
raze_always_inline i32 bsr_clz(T v) noexcept {
    constexpr auto digits = std::numeric_limits<T>::digits;
    ulong index;

    if constexpr (digits == 64) {
        raze_maybe_unused_attribute auto r = _BitScanReverse64(&index, static_cast<u32>(v));
        if constexpr (!Unsafe) { if (!r) return digits; }
    }
    else if constexpr (digits <= 32) {
        raze_maybe_unused_attribute auto r = _BitScanReverse(&index, static_cast<u32>(v));
        if constexpr (!Unsafe) { if (!r) return digits; }
    }

    return digits - 1 - index;
}

#endif // defined(raze_processor_x86)

#if defined(raze_processor_x86)

template <std::unsigned_integral T>
raze_always_inline i32 lzcnt_clz(T v) noexcept {
    constexpr auto digits = std::numeric_limits<T>::digits;
    if constexpr (digits == 64)  return raze_lzcnt_u64(static_cast<u64>(v));
    else if constexpr (digits == 32) return raze_lzcnt_u32(static_cast<u32>(v)) - (32 - digits);
    else return raze_lzcnt_u16(static_cast<u16>(v)) - (16 - digits);
}

#endif // defined(raze_processor_x86)

template <std::unsigned_integral T>
constexpr raze_always_inline i32 count_leading_zero_bits_impl(T v) noexcept {
#if defined(raze_processor_x86)
    if not consteval {
        if (arch::ProcessorFeatures::AVX2())
            return lzcnt_clz(v);
        else
            return bsr_clz<false>(v);
    }
    else
#endif // defined(raze_processor_x86)
    {
        return bit_hacks_clz(v);
    }
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe>
struct clz_n_bits_implementation {
    template <std::unsigned_integral T>
    constexpr raze_always_inline i32 operator()(T v) const noexcept {
        constexpr auto offset = raze_sizeof_in_bits(T) - Bits;

        if constexpr (vx::has_avx2<ISA>) return lzcnt_clz(v) - offset;
        else return bsr_clz<Unsafe>(v) - offset;
    }
};

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 clz_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);

    return clz_n_bits_implementation<ISA, Bits, Unsafe>()(v);
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe>
struct clz_not_n_bits_implementation {
    template <std::unsigned_integral T>
    constexpr raze_always_inline i32 operator()(T v) const noexcept {
        constexpr auto offset = raze_sizeof_in_bits(T) - Bits;
        constexpr auto mask = (Bits == raze_sizeof_in_bits(T)) ? math::max_limit<T>() : T((T(1) << Bits) - 1);

        v = T(~v);
        if constexpr (Bits < 8) v &= mask;

        if constexpr (vx::has_avx2<ISA>) return lzcnt_clz(v) - offset;
        else return bsr_clz<Unsafe>(v) - offset;
    }
};

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 clz_not_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);

    return clz_not_n_bits_implementation<ISA, Bits, Unsafe>()(v);
}

__RAZE_MATH_NAMESPACE_END
