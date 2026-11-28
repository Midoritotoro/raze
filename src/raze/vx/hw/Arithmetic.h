#pragma once 

#include <src/raze/vx/hw/configurable/arithmetic/Options.h>
#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/arithmetic/Sub.h>
#include <src/raze/vx/hw/configurable/arithmetic/Mul.h>
#include <src/raze/vx/hw/configurable/arithmetic/Div.h>
#include <src/raze/vx/hw/configurable/arithmetic/Rem.h>
#include <src/raze/vx/hw/configurable/arithmetic/Negate.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMin.h>
#include <src/raze/vx/hw/configurable/arithmetic/Clamp.h>
#include <src/raze/vx/hw/configurable/arithmetic/Convert.h>
#include <src/raze/vx/hw/configurable/arithmetic/Sqrt.h>
#include <src/raze/vx/hw/configurable/arithmetic/Rsqrt.h>
#include <src/raze/vx/hw/configurable/arithmetic/Rcp.h>
#include <src/raze/vx/hw/configurable/arithmetic/CopySign.h>
#include <src/raze/vx/hw/configurable/arithmetic/Nearest.h>
#include <src/raze/vx/hw/configurable/arithmetic/Floor.h>
#include <src/raze/vx/hw/configurable/arithmetic/Ceil.h>
#include <src/raze/vx/hw/configurable/arithmetic/Trunc.h>
#include <src/raze/vx/hw/configurable/arithmetic/Round.h>


#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Arithmetic.h>
#endif

__RAZE_VX_NAMESPACE_BEGIN


__RAZE_VX_NAMESPACE_END
