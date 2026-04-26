#pragma once

#include <raze/arch/CpuFeature.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, class _Type_>
constexpr inline auto __bitmask_bits_per_element_v = __has_avx512bw_support_v<_ISA_> ? 1 : ((sizeof(_Type_) == 2) ? 2 : 1);

__RAZE_VX_NAMESPACE_END
