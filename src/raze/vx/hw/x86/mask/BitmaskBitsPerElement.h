#pragma once

#include <raze/arch/CpuFeature.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, class T>
constexpr inline auto bitmask_bits_per_element_v = has_avx512bw<ISA> ? 1 : ((sizeof(T) == 2) ? 2 : 1);

__RAZE_VX_NAMESPACE_END
