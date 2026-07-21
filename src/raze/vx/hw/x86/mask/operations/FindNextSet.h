#pragma once 

#include <src/raze/math/CountTrailingZeros.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_, bool _Unsafe_>
struct _Find_next_set {
    template <raw_mask_type _Tp_>
    raze_nodiscard raze_always_inline i32 operator()(_Tp_ __x, u32 __shift) const noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) {
            return __shift == 0 ? !__x : 1;
        }
        else {
            auto __mask = _To_mask<_ISA_, _Type_>()(__x);
            using _Mask_ = decltype(__mask);

            _Mask_ __mask_after = __shift == 0  ? __mask : _Mask_(__mask & _Mask_(_Mask_(~0) << __shift));
            return math::__ctz_n_bits<_ISA_, _Size_, _Unsafe_>(__mask_after);
        }
    }
};

__RAZE_VX_NAMESPACE_END
