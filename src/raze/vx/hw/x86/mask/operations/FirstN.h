#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <array>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _VectorLength_, arithmetic_type _Type_>
consteval auto first_n_vtable() noexcept {
    using _MaskType = typename IntegerForSizeof<_Type_>::Unsigned;
    auto table = std::array<_MaskType, _VectorLength_ * 2>{};

    for (auto i = 0; i < _VectorLength_; ++i)
        table[i] = ~_MaskType(0);

    for (auto i = _VectorLength_; i < _VectorLength_ * 2; ++i)
        table[i] = 0;

    return table;
}

template <arch::ISA	_ISA_, u32 _Size_, raw_mask_type _Tp_, arithmetic_type _Type_>
struct _First_n {
    raze_nodiscard raze_always_inline auto operator()(u32 elements) const noexcept {
        constexpr auto kmask = (has_avx512f<_ISA_> && sizeof(_Type_) >= 4) || (has_avx512bw<_ISA_>);

        if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
            return elements != 0;
        }
        else if constexpr (kmask) {
            if constexpr (has_bmi2<_ISA_>) {
                if constexpr (sizeof(_Tp_) == 1) return static_cast<_Tp_>(_bzhi_u32(0xFF, elements));
                else if constexpr (sizeof(_Tp_) == 2) return static_cast<_Tp_>(_bzhi_u32(0xFFFF, elements));
                else if constexpr (sizeof(_Tp_) == 4) return static_cast<_Tp_>(_bzhi_u32(0xFFFFFFFF, elements));
                else if constexpr (sizeof(_Tp_) == 8) return static_cast<_Tp_>(_bzhi_u64(0xFFFFFFFFFFFFFFFFULL, elements));
            }
            else {
                if constexpr (_Size_ == raze_sizeof_in_bits(_Tp_)) {
                    auto r = _Tp_((_Tp_(1) << elements) - 1);
                    return elements == _Size_ ? math::max_limit<_Tp_>() : r;
                }
                else {
                    return _Tp_((_Tp_(1) << elements) - 1);
                }
            }
        }
        else {
            static constexpr auto vtable = first_n_vtable<_Size_, _Type_>();
            auto* raze_restrict addr = algorithm::bytes_pointer_offset(vtable.data(), sizeof(_Tp_) - (elements * sizeof(_Type_)));
            const auto loaded = _Load<_ISA_, _Tp_>()(addr);
            return loaded;
        }
    }
};

template <sizetype _Bits_, class _Type_>
consteval auto max_for_bits() noexcept {
    if constexpr (_Bits_ == raze_sizeof_in_bits(_Type_)) return _Type_(-1);
    else return _Type_(_Type_(_Type_(1) << _Bits_) - 1);
}

__RAZE_VX_NAMESPACE_END
