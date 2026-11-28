#pragma once 

#include <src/raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/configurable/compare/IsLess.h>
#  include <src/raze/vx/hw/configurable/compare/IsLessEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsGreater.h>
#  include <src/raze/vx/hw/configurable/compare/IsGreaterEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsNotEqual.h>
#  include <src/raze/vx/hw/configurable/compare/IsNan.h>
#  include <src/raze/vx/hw/configurable/compare/IsInf.h>
#  include <src/raze/vx/hw/configurable/compare/IsFinite.h>
#  include <src/raze/vx/hw/configurable/compare/IsNotFinite.h>
#  include <src/raze/vx/hw/configurable/compare/IsNormal.h>
#  include <src/raze/vx/hw/configurable/compare/IsSubnormal.h>
#  include <src/raze/vx/hw/configurable/compare/IsZero.h>
#  include <src/raze/vx/hw/configurable/compare/SignBit.h>
#  include <src/raze/vx/hw/configurable/compare/IsUnordered.h>
#endif
