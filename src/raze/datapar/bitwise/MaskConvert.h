#pragma once 

#include <src/raze/datapar/bitwise/ToIndexMask.h>
#include <src/raze/datapar/bitwise/ToVector.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    class       _DesiredType_,
    class       _ToType_,
    class       _FromType_>
raze_always_inline _ToType_ __mask_convert(_FromType_ __from) noexcept {
    if constexpr (std::is_integral_v<_FromType_> && std::is_integral_v<_ToType_>)
        return static_cast<_ToType_>(__from);

    else if constexpr (__is_intrin_type_v<_FromType_> && __is_intrin_type_v<_ToType_>)
        return __intrin_bitcast<_ToType_>(__from);

    else if constexpr (__is_intrin_type_v<_FromType_> && std::is_integral_v<_ToType_>)
        return _Simd_to_mask<_ISA_, _Width_, _DesiredType_>()(__from);

    else if constexpr (std::is_integral_v<_FromType_> && __is_intrin_type_v<_ToType_>)
        return _Simd_to_vector<_ISA_, _Width_, _ToType_, _DesiredType_>()(__from);
}

__RAZE_DATAPAR_NAMESPACE_END
