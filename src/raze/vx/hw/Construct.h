#pragma once 

#include <raze/compatibility/Compatibility.h>

#include <src/raze/vx/hw/configurable/construct/AllBits.h>
#include <src/raze/vx/hw/configurable/construct/Zero.h>
#include <src/raze/vx/hw/configurable/construct/Broadcast.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Construct.h>
#endif
