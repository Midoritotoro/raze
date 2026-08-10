#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <array>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _VectorLength_, arithmetic_type _Type_>
consteval auto __first_n_vtable() noexcept {
    using _MaskType = typename IntegerForSizeof<_Type_>::Unsigned;
    auto __table = std::array<_MaskType, _VectorLength_ * 2>{};

    for (auto __i = 0; __i < _VectorLength_; ++__i)
        __table[__i] = ~_MaskType(0);

    for (auto __i = _VectorLength_; __i < _VectorLength_ * 2; ++__i)
        __table[__i] = 0;

    return __table;
}

template <class _MaskType_>
raze_nodiscard raze_always_inline _MaskType_ __bzhi_mask_first_n(u32 __elements) noexcept {
    if constexpr (sizeof(_MaskType_) == 1) return static_cast<_MaskType_>(_bzhi_u32(0xFF, __elements));
    else if constexpr (sizeof(_MaskType_) == 2) return static_cast<_MaskType_>(_bzhi_u32(0xFFFF, __elements));
    else if constexpr (sizeof(_MaskType_) == 4) return static_cast<_MaskType_>(_bzhi_u32(0xFFFFFFFF, __elements));
    else if constexpr (sizeof(_MaskType_) == 8) return static_cast<_MaskType_>(_bzhi_u64(0xFFFFFFFFFFFFFFFFULL, __elements));
}

template <arch::ISA	_ISA_, u32 _Size_, raw_mask_type _Tp_, arithmetic_type _Type_>
struct _First_n {
    raze_nodiscard raze_always_inline auto operator()(u32 __elements) const noexcept {
        constexpr auto __kmask = (__has_avx512f_support_v<_ISA_> && sizeof(_Type_) >= 4) || (__has_avx512bw_support_v<_ISA_>);

        if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
            return __elements != 0;
        }
        else if constexpr (__kmask) {
            return __bzhi_mask_first_n<_Tp_>(__elements);
        }
        else {
            static constexpr auto __vtable = __first_n_vtable<_Size_, _Type_>();
            auto* raze_restrict __addr = algorithm::__bytes_pointer_offset(__vtable.data(), sizeof(_Tp_) - (__elements * sizeof(_Type_)));
            const auto __loaded = _Load<_ISA_, _Tp_>()(__addr);
            return __loaded;
        }
    }
};

template <sizetype _Bits_, class _Type_>
consteval auto __max_for_bits() noexcept {
    if constexpr (_Bits_ == raze_sizeof_in_bits(_Type_)) return _Type_(-1);
    else return _Type_(_Type_(_Type_(1) << _Bits_) - 1);
}

__RAZE_VX_NAMESPACE_END
