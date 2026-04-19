#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class		_Type_>
struct _Not_equal {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline auto operator()(
        _IntrinType_ __left,
        _IntrinType_ __right) const noexcept
    {
        const auto __compared = _Equal<_ISA_, _Width_, _Type_>()(__left, __right);
        using _ComparedType = decltype(__compared);

        if constexpr (std::is_integral_v<_ComparedType>)
            return ~__compared;
        else
            return _Not<_ISA_, _Width_, _Type_>()(__compared);
    }
};

__RAZE_VX_NAMESPACE_END
