#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <array>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype VectorLength, arithmetic_type T>
consteval auto first_n_vtable() noexcept {
    using MaskType = typename IntegerForSizeof<T>::Unsigned;
    auto table = std::array<MaskType, VectorLength * 2>{};

    for (auto i = 0; i < VectorLength; ++i)
        table[i] = ~MaskType(0);

    for (auto i = VectorLength; i < VectorLength * 2; ++i)
        table[i] = 0;

    return table;
}

template <arch::ISA	ISA, u32 N, raw_mask_type M, arithmetic_type T>
raze_always_inline auto operator()(u32 elements) noexcept {
    constexpr auto kmask = (has_avx512f<ISA> && sizeof(T) >= 4) || (has_avx512bw<ISA>);

    if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
        return elements != 0;
    }
    else if constexpr (kmask) {
        if constexpr (has_bmi2<ISA>) {
            if constexpr (sizeof(M) == 1) return static_cast<M>(_bzhi_u32(0xFF, elements));
            else if constexpr (sizeof(M) == 2) return static_cast<M>(_bzhi_u32(0xFFFF, elements));
            else if constexpr (sizeof(M) == 4) return static_cast<M>(_bzhi_u32(0xFFFFFFFF, elements));
            else if constexpr (sizeof(M) == 8) return static_cast<M>(_bzhi_u64(0xFFFFFFFFFFFFFFFFULL, elements));
        }
        else {
            if constexpr (N == raze_sizeof_in_bits(M)) {
                auto r = M((M(1) << elements) - 1);
                return elements == N ? math::max_limit<M>() : r;
            }
            else {
                return M((M(1) << elements) - 1);
            }
        }
    }
    else {
        static constexpr auto table = first_n_vtable<N, T>();
        return load_<ISA, M>(algorithm::bytes_pointer_offset(table.data(), sizeof(M) - (elements * sizeof(T))));
    }
}

template <sizetype Bits, class T>
consteval auto max_for_bits() noexcept {
    if constexpr (Bits == raze_sizeof_in_bits(T)) return T(-1);
    else return T(T(T(1) << Bits) - 1);
}

__RAZE_VX_NAMESPACE_END
