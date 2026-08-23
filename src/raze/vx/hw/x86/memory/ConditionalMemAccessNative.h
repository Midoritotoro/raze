#pragma 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/hw/x86/cast/SimdIntegralTypesCheck.h>

__RAZE_VX_NAMESPACE_BEGIN

#if defined(raze_processor_x86)
  template <arch::ISA _ISA_, class _Type_>
  concept native_conditional_memory_access = arithmetic_type<_Type_> && ((has_avx<_ISA_> && sizeof(_Type_) >= 4) || has_avx512bw<_ISA_>);
#endif

__RAZE_VX_NAMESPACE_END
