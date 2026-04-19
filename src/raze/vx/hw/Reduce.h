#pragma once 

#include <raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Reduce.h>
#endif

#include <src/raze/vx/hw/configurable/ConfigurableBinaryOperation.h>
#include <src/raze/vx/hw/configurable/ConfigurableUnaryOperation.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
using _Configurable_reduce_add = __configurable_unary_operation_type<_Reduce_add, _Options_>;

__RAZE_VX_NAMESPACE_END
