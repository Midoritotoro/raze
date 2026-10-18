#pragma once 

#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/arithmetic/Sub.h>
#include <src/raze/vx/hw/configurable/arithmetic/Mul.h>
#include <src/raze/vx/hw/configurable/arithmetic/Div.h>
#include <src/raze/vx/hw/configurable/arithmetic/Negate.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMin.h>
#include <src/raze/vx/hw/configurable/arithmetic/Clamp.h>
#include <src/raze/vx/hw/configurable/arithmetic/Convert.h>


#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Arithmetic.h>
#endif

__RAZE_VX_NAMESPACE_BEGIN


__RAZE_VX_NAMESPACE_END
