#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <array>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype VectorLength, arithmetic_type T>
consteval auto first_n_vtable() noexcept {
    using MaskType = typename IntegerForSizeof<T>::Unsigned;
    auto table = std::array<MaskType, VectorLength * 2>{};

    for (sizetype i = 0; i < VectorLength; ++i)
        table[i] = ~MaskType(0);

    for (sizetype i = VectorLength; i < VectorLength * 2; ++i)
        table[i] = 0;

    return table;
}

template <sizetype Bits, class T>
consteval auto max_for_bits() noexcept {
    if constexpr (Bits == raze_sizeof_in_bits(T)) return static_cast<T>(~T(0));
    else return static_cast<T>((T(1) << Bits) - 1);
}

template <arch::ISA	ISA, u32 N, raw_mask_type M, arithmetic_type T>
raze_always_inline auto first_n_(u32 elements) noexcept {
    const auto clamped = elements > N ? N : elements;

    if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
        return clamped != 0;
    }
    else if constexpr (std::integral<M>) {
        constexpr auto full_mask = max_for_bits<N, M>();

        if constexpr (has_bmi2<ISA>) {
            if constexpr (sizeof(M) <= 4) return static_cast<M>(_bzhi_u32(static_cast<u32>(full_mask), clamped));
            else return static_cast<M>(_bzhi_u64(static_cast<u64>(full_mask), clamped));
        }
        else {
            if (clamped >= N)
                return full_mask;
            return static_cast<M>((M(1) << clamped) - 1);
        }
    }
    else {
        static constexpr auto table = first_n_vtable<N, T>();
        return load_<ISA, M>(algorithm::bytes_pointer_offset(table.data(), sizeof(M) - (clamped * sizeof(T))));
    }
}

__RAZE_VX_NAMESPACE_END
