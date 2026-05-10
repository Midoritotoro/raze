#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <raze/algorithm/minmax/Min.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <src/raze/vx/hw/x86/mask/operations/ToK.h>
#include <array>


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _VectorLength_>
consteval auto __first_n_ktable() noexcept {
    using _MaskType = __mmask_for_elements_t<_VectorLength_>;
    auto __table = std::array<_MaskType, _VectorLength_ + 1>{};

    for (auto __i = 0; __i <= _VectorLength_; ++__i) {
        if (__i == _VectorLength_) {
            if (_VectorLength_ >= 8)
                __table[__i] = math::__maximum_integral_limit<_MaskType>();
            else
                __table[__i] = _MaskType((_MaskType(1) << __i) - 1);
        }
        else {
            __table[__i] = _MaskType((_MaskType(1) << __i) - 1);
        }
    }

    return __table;
}

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

template <arch::ISA	_ISA_, u32 _Size_, raw_mask_type _Tp_, arithmetic_type _Type_>
struct _First_n {
    raze_nodiscard raze_always_inline auto operator()(u32 __elements) const noexcept {
        constexpr auto __kmask = (__has_avx512f_support_v<_ISA_> && sizeof(_Type_) >= 4) || (__has_avx512bw_support_v<_ISA_>);

        if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
            return __elements != 0;
        }
        else if constexpr (__kmask) {
#if 0
            constexpr auto __ktable = __first_n_ktable<_Size_>();
            return __ktable[__elements];
#else
           
            return __to_k<_ISA_>(_Tp_((math::__maximum_integral_limit<_Tp_>()) >> (_Size_ - __elements)));
#endif
        }
        else {
            constexpr auto __vtable = __first_n_vtable<_Size_, _Type_>();
            return _Load<_ISA_, _Tp_>()(algorithm::__bytes_pointer_offset(__vtable.data(), 64 - (__elements * sizeof(_Type_))));
        }
    }
};

__RAZE_VX_NAMESPACE_END
