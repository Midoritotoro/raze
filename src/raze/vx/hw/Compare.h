#pragma once 

#include <raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/configurable/compare/IsLess.h>
#  include <src/raze/vx/hw/configurable/compare/IsLessEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsGreater.h>
#  include <src/raze/vx/hw/configurable/compare/IsGreaterEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsNotEqual.h>
#endif
