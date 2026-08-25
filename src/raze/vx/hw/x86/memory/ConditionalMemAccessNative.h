#pragma 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/hw/x86/cast/SimdIntegralTypesCheck.h>

__RAZE_VX_NAMESPACE_BEGIN

#if defined(raze_processor_x86)
  template <arch::ISA ISA, class T>
  concept native_conditional_memory_access = arithmetic_type<T> && ((has_avx<ISA> && sizeof(T) >= 4) || has_avx512bw<ISA>);
#endif

__RAZE_VX_NAMESPACE_END
