#pragma once 

#include <src/raze/math/PopulationCount.h>

__RAZE_MATH_NAMESPACE_BEGIN

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
    else {
        v = v | (v >> 1);
        v = v | (v >> 2);
        v = v | (v >> 4);
        return bit_hacks_population_count(static_cast<T>(~v));
    }
}

template <std::unsigned_integral T>
raze_always_inline i32 lzcnt_clz(T v) noexcept {
    constexpr auto digits = std::numeric_limits<T>::digits;

#if defined(raze_cpp_msvc_only)
    if constexpr (digits == 64) return static_cast<i32>(__lzcnt64(static_cast<u64>(v)));
    else return static_cast<i32>(__lzcnt(static_cast<u32>(v))) - (32 - digits);
#else
    if constexpr (digits == 64) return v ? static_cast<i32>(__builtin_clzll(static_cast<unsigned long long>(v))) : 64;
    else return v ? static_cast<i32>(__builtin_clz(static_cast<unsigned int>(v))) - (32 - digits) : digits;
#endif
}

template <arch::ISA ISA, sizetype Bits, bool Unsafe>
struct clz_n_bits_implementation {
    template <std::unsigned_integral T>
    constexpr raze_always_inline i32 operator()(T v) const noexcept {
        static_assert(Bits <= 64);
        static_assert(raze_sizeof_in_bits(T) >= Bits);

        constexpr auto digits = std::numeric_limits<T>::digits;

#if defined(raze_cpp_msvc_only)
        if constexpr (vx::has_bmi1<ISA>) {
            if constexpr (Bits <= 32) return static_cast<i32>(__lzcnt(static_cast<u32>(v))) - (32 - Bits);
            else return static_cast<i32>(__lzcnt64(static_cast<u64>(v)));
        }
        else {
            ulong index;

            if constexpr (Bits <= 32) {
                raze_maybe_unused_attribute auto r = _BitScanReverse(&index, static_cast<u32>(v));
                if constexpr (!Unsafe) { if (!r) return static_cast<i32>(Bits); }
            }
            else {
                raze_maybe_unused_attribute auto r = _BitScanReverse64(&index, static_cast<u64>(v));
                if constexpr (!Unsafe) { if (!r) return static_cast<i32>(Bits); }
            }

            return static_cast<i32>(Bits) - 1 - static_cast<i32>(index);
        }
#else
        if constexpr (!Unsafe) {
            if (v == 0) return static_cast<i32>(Bits);
        }

        if constexpr (digits == 64) return static_cast<i32>(__builtin_clzll(static_cast<unsigned long long>(v))) - (64 - static_cast<i32>(Bits));
        else return static_cast<i32>(__builtin_clz(static_cast<unsigned int>(v))) - (32 - static_cast<i32>(Bits));
#endif
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
        static_assert(Bits <= 64);
        static_assert(raze_sizeof_in_bits(T) >= Bits);

        constexpr auto T_bits = raze_sizeof_in_bits(T);
        constexpr T mask = (Bits == T_bits) ? math::max_limit<T>() : static_cast<T>((T(1) << (Bits - 1) << 1) - 1);

        v = static_cast<T>(~v);
        if constexpr (Bits < 8) v &= mask;
        return clz_n_bits<ISA, Bits, Unsafe>(v);
    }
};

template <arch::ISA ISA, sizetype Bits, bool Unsafe, std::unsigned_integral T>
constexpr raze_always_inline i32 clz_not_n_bits(T v) noexcept {
    static_assert(Bits <= 64);
    static_assert(raze_sizeof_in_bits(T) >= Bits);

    return clz_not_n_bits_implementation<ISA, Bits, Unsafe>()(v);
}

__RAZE_MATH_NAMESPACE_END