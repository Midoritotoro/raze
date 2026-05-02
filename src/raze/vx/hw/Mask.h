#pragma once 

#include <raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Mask.h>
#  include <src/raze/vx/hw/configurable/mask/AllOf.h>
#  include <src/raze/vx/hw/configurable/mask/AnyOf.h>
#  include <src/raze/vx/hw/configurable/mask/NoneOf.h>
#  include <src/raze/vx/hw/configurable/mask/SomeOf.h>
#endif
